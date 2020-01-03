#pragma once
#include "fs.h"
#include <windows.h>
#include "part.h"

class Cache;
class BitVector;

class KernelFS {
	int FCBCnt;
	Partition* p;
	Cache* cache;
	BitVector* bitVect;

	static LONG volatile isInit;
	static CRITICAL_SECTION KernelFS_CS;
	static CONDITION_VARIABLE alreadyMounted;
	static CONDITION_VARIABLE openFilesExist;

public:
	friend class FS;
	static KernelFS* volatile mounted;
	static char mount(Partition* partition); // Create the cache object, load bitVect
	static char unmount(); // delete the cache object

	ClusterNo alloc();
	void dealloc(ClusterNo);

	char format();
	FileCnt readRootDir();
	char doesExist(char* fname);

	File* open(char* fname, char mode);
	char deleteFile(char* fname);
	KernelFS(Partition* p);
	~KernelFS();
};