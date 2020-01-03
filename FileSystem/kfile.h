#pragma once
#include "file.h"
#include "part.h"


class KernelFile
{
	ClusterNo ind1Adr, ind2Adr;
	int ind1Cursor, ind2Cursor;
	ClusterNo ind1[ClusterSize / sizeof(ClusterNo)];
	ClusterNo ind2[ClusterSize / sizeof(ClusterNo)];
	char data[ClusterSize];

	BytesCnt cursor;

public:
	char write(BytesCnt, char* buffer);
	BytesCnt read(BytesCnt, char* buffer);
	char seek(BytesCnt);
	BytesCnt filePos();
	char eof();
	BytesCnt getFileSize();
	char truncate();
	KernelFile(char m);
	~KernelFile();
};

