#include "kfile.h"
#include "kfs.h"
#include "cache.h"


KernelFile::KernelFile(DirDesc& dd, char m)
{
	this->ind1Adr = ind1Adr;
	this->mode = m;
	this->sizeOfFile = dd.size;
	cursorLoaded = 0;

}


KernelFile::~KernelFile()
{
}


char KernelFile::write(BytesCnt cnt, char* buffer) {
	for (unsigned i = 0; i < cnt; i++) {
		if (eof())
			if (!expand())
				return 0;
		data[dataCursor] = buffer[i];
		seek(cursor + 1);
	}
	return 1;
}

BytesCnt KernelFile::read(BytesCnt cnt, char* buffer) {
	for (unsigned i = 0; i < cnt; i++) {
		if (eof())
			return i;
		buffer[i] = data[dataCursor];
		seek(cursor + 1);
	}
	return cnt;
}

char KernelFile::seek(BytesCnt r) {
	if (r > getFileSize())
		return 0;
	int A = ClusterSize / sizeof(ClusterNo);
	int B = A;
	int C = ClusterSize;
	int a = r / (B*C); // ind1Pointer
	int b = (r - a*B*C) / C; //ind2Pointer
	int c = r - a*B*C - b*C; //dataBytePointer

	if (a != ind1Cursor || !cursorLoaded) {
		if (cursorLoaded) {
			KernelFS::mounted->cache->writeCluster(ind2[ind2Cursor], (char*)data);
			KernelFS::mounted->cache->writeCluster(ind1[ind1Cursor], (char*)ind2);
		}
		KernelFS::mounted->cache->readCluster(ind1[a], (char*)ind2);
		KernelFS::mounted->cache->readCluster(ind2[b], (char*)data);
	}
	else {
		if (b != ind2Cursor) {
			if(cursorLoaded)
				KernelFS::mounted->cache->writeCluster(ind2[ind2Cursor], (char*)data);
			KernelFS::mounted->cache->readCluster(ind2[b], (char*)data);
		}
	}
	ind1Cursor = a;
	ind2Cursor = b;
	dataCursor = c;
	cursor = r;
	cursorLoaded = 1;
	return 1;
}

BytesCnt KernelFile::filePos() {
	return cursor;
}

char KernelFile::eof() {
	if (cursor > getFileSize())
		exit(1234); //check
	return cursor == getFileSize();
}

BytesCnt KernelFile::getFileSize() {
	return sizeOfFile;
}

char KernelFile::truncate() {
	const int entries = ClusterSize / sizeof(ClusterNo);
	int r = cursor;
	int A = entries;
	int B = A;
	int C = ClusterSize;
	int a = r / (B*C); // ind1Pointer
	int b = (r - a * B*C) / C; //ind2Pointer
	int c = r - a * B*C - b * C; //dataBytePointer
	

	b += 1;
	while (b < entries && ind2[b] != (ClusterNo)0) {
		KernelFS::mounted->dealloc(ind2[b]);
		ind2[b] = 0;
		b++;
	}


	ClusterNo ind2[entries]; // ind2 now refers to a local version not this->ind2
	a += 1;
	while (a < entries && ind1[a] != (ClusterNo)0) {
		b = 0;
		KernelFS::mounted->cache->readCluster(ind1[a], (char*)ind2);
		while (b < entries && ind2[b] != (ClusterNo)0) {
			KernelFS::mounted->dealloc(ind2[b]);
			b++;
		}
		KernelFS::mounted->dealloc(ind1[a]);
		ind1[a] = 0;
		a++;
	}
	sizeOfFile = cursor;
	return 1;
}

char KernelFile::expand()
{
	int r = getFileSize();
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
			memset(ind2, 0, ClusterSize);
		}
		ClusterNo newData = KernelFS::mounted->alloc();
		ind2[b] = newData;
		memset(data, 0, ClusterSize);
	}
	this->sizeOfFile++;
}