#pragma once
#include <stdint.h>
#include "fs.h"
#include <windows.h>
#include "part.h"
#include <string>
#include <map>

extern class Cache;
extern class BitVector;
extern class Directory;

struct DirDesc {
	char name[8];
	char ext[3];
	char zero;
	uint32_t ind1;
	uint32_t size;
	char rest[12];
};

class KernelFS {
	int FCBCnt;
	int isFormating;
	Partition* p;
	Cache* cache;
	BitVector* bitVect;
	Directory* dir;
	std::map<std::string, PSRWLOCK> openFileTable;


	static LONG volatile isInit;
	static CRITICAL_SECTION KernelFS_CS;
	static CONDITION_VARIABLE alreadyMounted;
	static CONDITION_VARIABLE openFilesExist;

public:
	friend class FS;
	friend class KernelFile;
	friend class Directory;
	static KernelFS* volatile mounted;
	static char mount(Partition* partition); // Create the cache object, load bitVect
	static char unmount(); // delete the cache object

	ClusterNo alloc();
	void dealloc(ClusterNo);
	void dealloc(ClusterNo targets[], int cnt);

	char format();
	FileCnt readRootDir();
	char doesExist(char* fname);

	File* open(char* fname, char mode);
	char deleteFile(char* fname);
	KernelFS(Partition* p);
	~KernelFS();
};