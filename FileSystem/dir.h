#pragma once
#include "kfs.h"
class KernelFile;

class Directory {
	KernelFile* kf;
public:
	Directory(ClusterNo);
	~Directory();
	char getDirDesc(char* fileName, DirDesc* desc, int& i);
	char addDirDesc(DirDesc* desc);
	char clearDirDesc(int i);
	FileCnt cntFiles();
};