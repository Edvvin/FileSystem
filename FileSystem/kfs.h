#pragma once
#include "fs.h"

class KernelFS {
public:
	~KernelFS();
	char mount(Partition* partition);
	char unmount();
	char format();
	FileCnt readRootDir();
	char doesExist(char* fname);

	File* open(char* fname, char mode);
	char deleteFile(char* fname);
	KernelFS();
};