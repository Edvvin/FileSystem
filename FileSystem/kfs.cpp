#include "kfs.h"
#include <stdlib.h>
#include "part.h"
#include "cache.h"
#include "bitvect.h"
#include "dir.h"
KernelFS* volatile KernelFS::mounted = NULL;
LONG volatile KernelFS::isInit = 0;


KernelFS::KernelFS(Partition* p) {
	FCBCnt = 0;
	this->p = p;
	cache = new Cache(p);
	bitVect = new BitVector(cache);
	dir = new Directory(this);
}

KernelFS::~KernelFS() {
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
	bitVect->writeThrough();


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
	bitVect->writeThrough();
	LeaveCriticalSection(&KernelFS_CS);
}

void KernelFS::dealloc(ClusterNo targets[], int cnt)
{
	if (!isInit) {
		return;
	}
	EnterCriticalSection(&KernelFS_CS);
	if (mounted == NULL) {
		LeaveCriticalSection(&KernelFS_CS);
		return;
	}
	for (int i = 0; i < cnt; i++) {
		bitVect->reset(targets[i]);
	}
	bitVect->writeThrough();
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
	delete dir;
	char* emptyCluster = new char[ClusterSize];
	memset(emptyCluster, 0, ClusterSize);
	bitVect->clear();
	unsigned i;
	for (i = 0; i < cache->getNumOfClusters() / ClusterSize; i++)
	{
		cache->writeCluster(i, emptyCluster);
		bitVect->set(i);
	}
	
	cache->writeCluster(i, emptyCluster);
	bitVect->set(i);
	bitVect->writeThrough();
	delete [] emptyCluster;
	cache->sync();
	dir = new Directory(this);
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

	if (dir->getDirDesc(fname, &dd))
		ansr = 1;
	else ansr = 0;

	LeaveCriticalSection(&KernelFS_CS);
	return ansr;
}

File* KernelFS::open(char* fname, char mode) {
	if (!isInit) {
		return 0;
	}
	EnterCriticalSection(&KernelFS_CS);
	if (mounted == NULL) {
		LeaveCriticalSection(&KernelFS_CS);
		return 0;
	}

	// CODE GOES HERE

	LeaveCriticalSection(&KernelFS_CS);
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

	// CODE GOES HERE

	LeaveCriticalSection(&KernelFS_CS);
}

