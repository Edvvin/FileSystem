#pragma once
#include "file.h"
#include "part.h"
#include "kfs.h"

class KernelFile
{
	ClusterNo ind1Adr;
	int ind1Cursor, ind2Cursor, dataCursor;
	ClusterNo ind1[ClusterSize / sizeof(ClusterNo)];
	ClusterNo ind2[ClusterSize / sizeof(ClusterNo)];
	char data[ClusterSize];
	int dirtyData, dirtyInd1, dirtyInd2;
	BytesCnt cursor;
	BytesCnt sizeOfFile;
	int fileInd;
	char mode;
	int cursorLoaded;
	DirDesc dd;
	char expand();
public:
	char write(BytesCnt, char* buffer);
	BytesCnt read(BytesCnt, char* buffer);
	char seek(BytesCnt);
	BytesCnt filePos();
	char eof();
	BytesCnt getFileSize();
	char truncate();
	KernelFile(DirDesc& dd, int fileInd, char m);
	~KernelFile();
};