#pragma once
#include "kfs.h"
extern class KernelFile;

class Directory {
	KernelFile* kf;
public:
	Directory(KernelFS* fs);
	~Directory();
	char getDirDesc(char* fileName, DirDesc* desc);
	FileCnt cntFiles();
};