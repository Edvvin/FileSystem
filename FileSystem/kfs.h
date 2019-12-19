#pragma once
#include "fs.h"

class KernelFS {
public:
	~KernelFS();
	static char mount(Partition* partition, KernelFS*& kfs);
	static char unmount(KernelFS*& kfs);
	char format();
	FileCnt readRootDir();
	char doesExist(char* fname);

	File* open(char* fname, char mode);
	char deleteFile(char* fname);
	KernelFS();
};