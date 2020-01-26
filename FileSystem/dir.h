#pragma once
#include "kfs.h"
class KernelFile;

class Directory {
	ClusterNo ind1Adr;
	int ind1Cursor, ind2Cursor, dataCursor;
	ClusterNo ind1[ClusterSize / sizeof(ClusterNo)];
	ClusterNo ind2[ClusterSize / sizeof(ClusterNo)];
	char data[ClusterSize];
	int dirtyData, dirtyInd1, dirtyInd2;
	BytesCnt cursor;
	BytesCnt sizeOfFile;
	int cursorLoaded;

	char seek(BytesCnt);
	char expand(BytesCnt lastByte);
public:
	Directory(ClusterNo);
	~Directory();
	DirDesc getDirDesc(int i);
	void setDirDesc(int i, DirDesc& dd);
	char eof(int i);
};