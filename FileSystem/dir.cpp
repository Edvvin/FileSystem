#include "dir.h"
#include "kfile.h"
#include "cache.h"


char Directory::seek(BytesCnt r)
{
	int A = ClusterSize / sizeof(ClusterNo);
	int B = A;
	int C = ClusterSize;
	int a = r / (B*C); // ind1Pointer
	int b = (r - a * B*C) / C; //ind2Pointer
	int c = r - a * B*C - b * C; //dataBytePointer

	if (!cursorLoaded || a != ind1Cursor) {
		if (cursorLoaded) {
			if (dirtyData)
				KernelFS::mounted->cache->writeCluster(ind2[ind2Cursor], (char*)data);
			if (dirtyInd2)
				KernelFS::mounted->cache->writeCluster(ind1[ind1Cursor], (char*)ind2);
		}
		KernelFS::mounted->cache->readCluster(ind1[a], (char*)ind2);
		KernelFS::mounted->cache->readCluster(ind2[b], (char*)data);
		cursorLoaded = 1;
		dirtyData = 0;
		dirtyInd2 = 0;
	}
	else if (b != ind2Cursor) {
		if (dirtyData)
			KernelFS::mounted->cache->writeCluster(ind2[ind2Cursor], (char*)data);
		KernelFS::mounted->cache->readCluster(ind2[b], (char*)data);
		dirtyData = 0;
	}

	ind1Cursor = a;
	ind2Cursor = b;
	dataCursor = c;
	cursor = r;
	return 1;
}

Directory::Directory(ClusterNo ind1Adr)
{
	this->ind1Adr = ind1Adr;
	cursorLoaded = 0;
	dirtyData = 0;
	dirtyInd2 = 0;
	dirtyInd1 = 0;
}

Directory::~Directory()
{
	if (dirtyData)
		KernelFS::mounted->cache->writeCluster(ind2[ind2Cursor], (char*)data);
	if (dirtyInd2)
		KernelFS::mounted->cache->writeCluster(ind1[ind1Cursor], (char*)ind2);
	if (dirtyInd1)
		KernelFS::mounted->cache->writeCluster(ind1Adr, (char*)ind1);
}

DirDesc Directory::getDirDesc(int i)
{
	DirDesc dd;
	char* buffer = (char*)&dd;
	seek(i* sizeof(DirDesc));
	for (BytesCnt i = 0; i < sizeof(DirDesc); i++) {
		buffer[i] = data[dataCursor];
		seek(cursor + 1);
	}
	return dd;
}

void Directory::setDirDesc(int i, DirDesc & dd)
{
	if (eof(i))
		expand(i*sizeof(DirDesc));
	char* buffer = (char*)&dd;
	seek(i * sizeof(DirDesc));
	for (BytesCnt i = 0; i < sizeof(DirDesc); i++) {
		data[dataCursor] = buffer[i];
		dirtyData = 1;
		seek(cursor + 1);
	}
	return;
}

char Directory::eof(int i)
{
	DirDesc zero;
	memset(&zero, 0, sizeof(DirDesc));
	seek(i * sizeof(ClusterNo));
	DirDesc dd = getDirDesc(i);
	return !memcmp(&dd, &zero, sizeof(DirDesc));
}



char Directory::expand(BytesCnt lastByte)
{
	int r = lastByte;
	for (int i = 0; i < sizeof(DirDesc); i++) {
		int A = ClusterSize / sizeof(ClusterNo);
		int B = A;
		int C = ClusterSize;
		int a = r / (B*C); // ind1Pointer
		int b = (r - a * B*C) / C; //ind2Pointer
		int c = r - a * B*C - b * C; //dataBytePointer
		if (c == 0) {
			if (b == 0) {
				ClusterNo newInd2 = KernelFS::mounted->alloc();
				if (newInd2 == 0)
					return 0;
				ind1[a] = newInd2;
				dirtyInd1 = 1;
				memset(ind2, 0, ClusterSize);
				dirtyInd2 = 1;
			}
			ClusterNo newData = KernelFS::mounted->alloc();
			ind2[b] = newData;
			dirtyInd2 = 1;
			dirtyData = 1;
			memset(data, 0, ClusterSize);
		}
		r++;
	}
	return 1;
}