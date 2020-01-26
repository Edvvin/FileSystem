#include "kfs.h"
#include "kfile.h"
#include <stdlib.h>
#include "part.h"
#include "cache.h"
#include "rcache.h"
#include "bitvect.h"
#include "dir.h"
KernelFS* volatile KernelFS::mounted = NULL;
LONG volatile KernelFS::isInit = 0;
CRITICAL_SECTION KernelFS::KernelFS_CS;
CONDITION_VARIABLE KernelFS::alreadyMounted;
CONDITION_VARIABLE KernelFS::openFilesExist;

KernelFS::KernelFS(Partition* p) {
	FCBCnt = 0;
	isFormating = 0;
	this->p = p;
	cache = new RealCache(p, p->getNumOfClusters() / 10);
	bitVect = new BitVector(p);
	dir = new Directory(bitVect->size());
}

KernelFS::~KernelFS() {
	delete dir;
	delete bitVect;
	delete cache;
}

char KernelFS::mount(Partition* partition) {
	if (!InterlockedExchange(&isInit, 1)) {
		InitializeCriticalSection(&KernelFS_CS);
		InitializeConditionVariable(&alreadyMounted);
		InitializeConditionVariable(&openFilesExist);
	}
	EnterCriticalSection(&KernelFS_CS);
	if (mounted != NULL) {
		SleepConditionVariableCS(&alreadyMounted, &KernelFS_CS, INFINITE);
	}
	mounted = new KernelFS(partition);
	LeaveCriticalSection(&KernelFS_CS);
	return 1;
}

char KernelFS::unmount() {
	if (!isInit) {
		return 0;
	}
	EnterCriticalSection(&KernelFS_CS);
	if (mounted == NULL) {
		LeaveCriticalSection(&KernelFS_CS);
		return 0;
	}

	while (mounted->FCBCnt) {
		SleepConditionVariableCS(&openFilesExist, &KernelFS_CS, INFINITE); // awake when FCBCnt == 0
	}

	if (mounted != NULL) {
		delete mounted;
		mounted = NULL;
		LeaveCriticalSection(&KernelFS_CS);
		WakeConditionVariable(&alreadyMounted);
	}
	else
		LeaveCriticalSection(&KernelFS_CS);
	return 1;
}

ClusterNo KernelFS::alloc()
{
	if (!isInit) {
		return 0;
	}
	EnterCriticalSection(&KernelFS_CS);
	if (mounted == NULL) {
		LeaveCriticalSection(&KernelFS_CS);
		return 0;
	}

	ClusterNo freeCluster = bitVect->find();
	bitVect->set(freeCluster);


	LeaveCriticalSection(&KernelFS_CS);
	return freeCluster;
}

void KernelFS::dealloc(ClusterNo target)
{
	if (!isInit) {
		return;
	}
	EnterCriticalSection(&KernelFS_CS);
	if (mounted == NULL) {
		LeaveCriticalSection(&KernelFS_CS);
		return;
	}
	if (target == 0)
		exit(12345); // check
	bitVect->reset(target);
	LeaveCriticalSection(&KernelFS_CS);
}

char KernelFS::format() {
	if (!isInit) {
		return 0;
	}
	EnterCriticalSection(&KernelFS_CS);
	if (mounted == NULL) {
		LeaveCriticalSection(&KernelFS_CS);
		return 0;
	}
	isFormating = 1;
	if(FCBCnt > 0)
		SleepConditionVariableCS(&openFilesExist, &KernelFS_CS, INFINITE); // awake when FCBCnt == 0
	cache->clear(0); //TODO
	bitVect->clear();
	for (ClusterNo i = 0; i < bitVect->size(); i++)
	{
		bitVect->set(i);
	}
	char* emptyCluster = new char[ClusterSize];
	memset(emptyCluster, 0, ClusterSize);
	cache->writeCluster(bitVect->size(), emptyCluster);
	bitVect->set(bitVect->size());
	delete [] emptyCluster;
	dir->setSize(0);
	isFormating = 0;
	LeaveCriticalSection(&KernelFS_CS);
	return 1;
}
FileCnt KernelFS::readRootDir() {
	if (!isInit) {
		return 0;
	}
	FileCnt ansr;
	EnterCriticalSection(&KernelFS_CS);
	if (mounted == NULL) {
		LeaveCriticalSection(&KernelFS_CS);
		return 0;
	}

	ansr = dir->cntFiles();

	LeaveCriticalSection(&KernelFS_CS);
	return ansr;
}

char KernelFS::doesExist(char* fname) {
	if (!isInit) {
		return 0;
	}
	char ansr;
	EnterCriticalSection(&KernelFS_CS);
	if (mounted == NULL) {
		LeaveCriticalSection(&KernelFS_CS);
		return 0;
	}
	DirDesc dd;
	int ind;
	if (dir->getDirDesc(fname, &dd, ind))
		ansr = 1;
	else ansr = 0;

	LeaveCriticalSection(&KernelFS_CS);
	return ansr;
}

File* KernelFS::open(char* fname, char mode) {
	if (!fname)
		return 0;
	if (*fname != '/')
		return 0;
	if (!isInit) {
		return 0;
	}
	EnterCriticalSection(&KernelFS_CS);
	if (mounted == NULL) {
		LeaveCriticalSection(&KernelFS_CS);
		return 0;
	}
	if(isFormating) {
		LeaveCriticalSection(&KernelFS_CS);
		return 0;
	}
	DirDesc dd;
	int ind;
	char exists = dir->getDirDesc(fname, &dd, ind);
	if (mode == 'w') {
		if (!openFileTable.count(fname)) {
			PSRWLOCK psr = 0;
			InitializeSRWLock(psr);
			openFileTable[fname] = psr;
		}
		LeaveCriticalSection(&KernelFS_CS);
		AcquireSRWLockExclusive(openFileTable[fname]);
		if (!exists) {
			char* name = fname + 1;
			char* dot = strchr(name, '.');
			int n = (dot - name) < FNAMELEN? (dot - name) : FNAMELEN;
			strncpy(dd.name, name, n);
			strncpy(dd.ext, dot + 1, FEXTLEN);
			dd.size = 0;
			dd.ind1 = alloc();
			EnterCriticalSection(&KernelFS_CS);
			dir->addDirDesc(&dd);
		}
		File* ret = new File();
		ret->myImpl = new KernelFile(dd, mode, fname);
		ret->myImpl->seek(0);
		if (!exists)
			ret->myImpl->truncate();
		FCBCnt++;
		return ret;
	}
	else if (mode = 'a') {
		if (!exists) {
			LeaveCriticalSection(&KernelFS_CS);
			return NULL;
		}
		if (!openFileTable.count(fname)) {
			PSRWLOCK psr = 0;
			InitializeSRWLock(psr);
			openFileTable[fname] = psr;
		}
		LeaveCriticalSection(&KernelFS_CS);
		AcquireSRWLockExclusive(openFileTable[fname]);
		File* ret = new File();
		ret->myImpl = new KernelFile(dd, mode, fname);
		ret->myImpl->seek(ret->myImpl->getFileSize());
		FCBCnt++;
		return ret;
	}
	else if (mode == 'r'){
		if (!exists) {
			LeaveCriticalSection(&KernelFS_CS);
			return NULL;
		}
		if (!openFileTable.count(fname)) {
			PSRWLOCK psr = 0;
			InitializeSRWLock(psr);
			openFileTable[fname] = psr;
		}
		LeaveCriticalSection(&KernelFS_CS);
		AcquireSRWLockShared(openFileTable[fname]);
		File* ret = new File();
		ret->myImpl = new KernelFile(dd, mode, fname);
		ret->myImpl->seek(0);
		FCBCnt++;
		return ret;
	}

	LeaveCriticalSection(&KernelFS_CS);
	return NULL;
}

char KernelFS::deleteFile(char* fname) {
	if (!isInit) {
		return 0;
	}
	EnterCriticalSection(&KernelFS_CS);
	if (mounted == NULL) {
		LeaveCriticalSection(&KernelFS_CS);
		return 0;
	}
	if (!openFileTable.count(fname)) {
		LeaveCriticalSection(&KernelFS_CS);
		return 0;
	}
	DirDesc dd;
	int ind;
	char exists = dir->getDirDesc(fname, &dd, ind);
	if (!exists) {
		LeaveCriticalSection(&KernelFS_CS);
		return 0;
	}
	KernelFile* f = new KernelFile(dd, 'w', fname);
	f->truncate();
	delete f;
	LeaveCriticalSection(&KernelFS_CS);
	dealloc(dd.ind1);
	EnterCriticalSection(&KernelFS_CS);
	dir->clearDirDesc(ind);
	LeaveCriticalSection(&KernelFS_CS);
	return 1;
}

