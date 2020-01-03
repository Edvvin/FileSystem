#include "kfs.h"
#include <stdlib.h>
#include "part.h"
#include "cache.h"
#include "bitvect.h"

KernelFS* volatile KernelFS::mounted = NULL;
LONG volatile KernelFS::isInit = 0;


KernelFS::KernelFS(Partition* p) {
	FCBCnt = 0;
	this->p = p;
	cache = new Cache(p);
	bitVect = new BitVector(cache);
}

KernelFS::~KernelFS() {
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
		SleepConditionVariableCS(&openFilesExist, &KernelFS_CS, INFINITE);
	}

	if (mounted != NULL) {
		delete mounted;
		mounted = NULL;
	}

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


	char* emptyCluster = new char[ClusterSize];
	memset(emptyCluster, 0, ClusterSize);


	cache->writeCluster(0, emptyCluster);
	cache->writeCluster(1, emptyCluster);
	bitVect->clear();
	bitVect->set(0);
	bitVect->set(1);
	bitVect->writeThrough();
	//TODO: probably smth related to the root dir
	LeaveCriticalSection(&KernelFS_CS);
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

	// CODE GOES HERE

	LeaveCriticalSection(&KernelFS_CS);
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

	// CODE GOES HERE

	LeaveCriticalSection(&KernelFS_CS);
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

