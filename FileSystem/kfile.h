#pragma once
#include "file.h"
#include "part.h"


class KernelFile
{
	ClusterNo ind1Adr;
	int ind1Cursor, ind2Cursor, dataCursor;
	ClusterNo ind1[ClusterSize / sizeof(ClusterNo)];
	ClusterNo ind2[ClusterSize / sizeof(ClusterNo)];
	char data[ClusterSize];

	BytesCnt cursor;
	BytesCnt sizeOfFile;

	char mode;
	int cursorLoaded;

public:
	char expand();
	char write(BytesCnt, char* buffer);
	BytesCnt read(BytesCnt, char* buffer);
	char seek(BytesCnt);
	BytesCnt filePos();
	char eof();
	BytesCnt getFileSize();
	char truncate();
	KernelFile(ClusterNo ind1Adr, char mode);
	~KernelFile();
};