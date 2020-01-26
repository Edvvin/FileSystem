#include "kfs.h"
#include "kfile.h"
#include <stdlib.h>
#include "part.h"
#include "cache.h"
#include "rcache.h"
#include "bitvect.h"
#include "dir.h"
#include "kfile.h"

KernelFS* volatile KernelFS::mounted = NULL;
LONG volatile KernelFS::isInit = 0;
CRITICAL_SECTION KernelFS::KernelFS_CS;
CONDITION_VARIABLE KernelFS::alreadyMounted;
CONDITION_VARIABLE KernelFS::openFilesExist;

KernelFS::KernelFS(Partition* p) {
	FCBCnt = 0;
	isFormating = 0;
	this->p = p;
	mounted = this;
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

	new KernelFS(partition); // this sets the mounted ptr

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
	static char zeros[ClusterSize];
	static int set = 0;
	if (set == 0) {
		set = 1;
		memset(zeros, 0, ClusterSize);
	}

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
	cache->writeCluster(freeCluster, zeros);
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
	cache->clear(0);
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
	isFormating = 0;
	LeaveCriticalSection(&KernelFS_CS);
	return 1;
}


FileCnt KernelFS::readRootDir() {
	if (!isInit) {
		return 0;
	}
	EnterCriticalSection(&KernelFS_CS);
	if (mounted == NULL) {
		LeaveCriticalSection(&KernelFS_CS);
		return 0;
	}
	int i = 0;
	FileCnt ansr = 0;
	char zeros[FNAMELEN];
	memset(zeros, 0, FNAMELEN);
	while (1) {
		if (dir->eof(i)) {
			break;
		}
		DirDesc dd = dir->getDirDesc(i);
		if (!memcmp(dd.name, zeros, FNAMELEN))
			ansr++;
		i++;
	}

	LeaveCriticalSection(&KernelFS_CS);
	return ansr;
}

char KernelFS::doesExist(char* fname) {
	if (!isInit) {
		return 0;
	}
	EnterCriticalSection(&KernelFS_CS);
	if (mounted == NULL) {
		LeaveCriticalSection(&KernelFS_CS);
		return 0;
	}
	char ansr = 0;
	int fileInd = 0;
	int exists = 0;
	dir->find(fname, fileInd, exists);
	if (exists == 1)
		ansr = 1;
	LeaveCriticalSection(&KernelFS_CS);
	return ansr;
}

File* KernelFS::open(char* fname, char mode) {
	if (!fname)
		return 0;
	if (!isInit) {
		return 0;
	}
	EnterCriticalSection(&KernelFS_CS);
	// almost all of the checks
	if (mounted == NULL) {
		LeaveCriticalSection(&KernelFS_CS);
		return 0;
	}
	if(isFormating) {
		LeaveCriticalSection(&KernelFS_CS);
		return 0;
	}
	if (mode != 'w' && mode != 'a' && mode != 'r') {
		LeaveCriticalSection(&KernelFS_CS);
		return 0;
	}
	if (fname[0] != '/') {
		LeaveCriticalSection(&KernelFS_CS);
		return 0;
	}

	int fileInd = 0;
	int exists = 1;
	dir->find(fname, fileInd, exists);
	if(exists < 0) {
		LeaveCriticalSection(&KernelFS_CS);
		return 0;
	}

	if(mode != 'w' && !exists){
		LeaveCriticalSection(&KernelFS_CS);
		return 0;
	}

	// setting up the FileTableEntry
	PSRWLOCK psr = 0;
	if (openFileTable.count(fileInd)) 
	{
		psr = openFileTable[fileInd]->lock;
	}
	else 
	{
		InitializeSRWLock(psr);
		openFileTable[fileInd] = new FileTableEntry(psr);
	}

	// setting up the dirdesc
	DirDesc dd;
	if(exists)
		dd = dir->getDirDesc(fileInd);

	LeaveCriticalSection(&KernelFS_CS);

	if (mode == 'w') {
		AcquireSRWLockExclusive(psr);
		if (!exists) {
			fileInd = dir->addFile(fname);
		}
		File* ret = new File();
		ret->myImpl = new KernelFile(dd, fileInd, mode);
		ret->myImpl->seek(0);
		if (exists)
			ret->myImpl->truncate();
		FCBCnt++;
		openFileTable[fileInd]->waitCnt++;
		return ret;
	}
	else if (mode = 'a') {
		AcquireSRWLockExclusive(psr);
		File* ret = new File();
		ret->myImpl = new KernelFile(dd, fileInd, mode);
		ret->myImpl->seek(ret->myImpl->getFileSize()); // TODO  think about this
		FCBCnt++;
		openFileTable[fileInd]->waitCnt++;
		return ret;
	}
	else if (mode == 'r'){
		AcquireSRWLockShared(psr);
		File* ret = new File();
		ret->myImpl = new KernelFile(dd, fileInd, mode);
		ret->myImpl->seek(0);
		FCBCnt++;
		openFileTable[fileInd]->waitCnt++;
		return ret;
	}
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

	DirDesc dd;
	int fileInd = 0;
	int exists = 0;

	dir->find(fname, fileInd, exists);

	if (exists != 1) {
		LeaveCriticalSection(&KernelFS_CS);
		return 0;
	}


	if (!openFileTable.count(fileInd)) {
		LeaveCriticalSection(&KernelFS_CS);
		return 0;
	}

	dd = dir->getDirDesc(fileInd);

	KernelFile* kf = new KernelFile(dd,fileInd,'w');
	kf->seek(0);
	kf->truncate();
	delete kf;
	memset(dd.name, 0, FNAMELEN);
	dir->setDirDesc(fileInd, dd);
	LeaveCriticalSection(&KernelFS_CS);
	return 1;
}

