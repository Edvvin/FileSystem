#include "kfile.h"
#include "kfs.h"
#include "cache.h"
#include "dir.h"

KernelFile::KernelFile(DirDesc& dd, int fileInd, char m)
{
	this->ind1Adr = dd.ind1;
	this->mode = m;
	this->sizeOfFile = dd.size;
	this->fileInd = fileInd;
	this->dd = dd;
	cursorLoaded = 0;
	dirtyData = 0;
	dirtyInd2 = 0;
	dirtyInd1 = 0;
	EnterCriticalSection(&KernelFS::mounted->KernelFS_CS);
	KernelFS::mounted->cache->readCluster(ind1Adr, (char*)ind1);
	LeaveCriticalSection(&KernelFS::mounted->KernelFS_CS);
}


KernelFile::~KernelFile()
{
	EnterCriticalSection(&KernelFS::mounted->KernelFS_CS);
	if (dirtyData)
		KernelFS::mounted->cache->writeCluster(ind2[ind2Cursor], (char*)data);
	if(dirtyInd2)
		KernelFS::mounted->cache->writeCluster(ind1[ind1Cursor], (char*)ind2);
	if (dirtyInd1)
		KernelFS::mounted->cache->writeCluster(ind1Adr, (char*)ind1);

	if (mode == 'a' || mode == 'w') {
		dd.size = sizeOfFile;
		KernelFS::mounted->dir->setDirDesc(fileInd, dd);
		if (--KernelFS::mounted->openFileTable[fileInd]->wCnt == 0)
			WakeAllConditionVariable(&KernelFS::rwLocked);
	}
	else {
		if (--KernelFS::mounted->openFileTable[fileInd]->rCnt == 0)
			WakeAllConditionVariable(&KernelFS::rwLocked);
	}

	if (KernelFS::mounted->openFileTable[fileInd]->wCnt == 0 &&
		KernelFS::mounted->openFileTable[fileInd]->rCnt == 0) {
		delete KernelFS::mounted->openFileTable[fileInd];
		KernelFS::mounted->openFileTable.erase(fileInd);
	}
	
	if (--KernelFS::mounted->FCBCnt == 0) {
		WakeConditionVariable(&KernelFS::mounted->openFilesExist);
	}
	LeaveCriticalSection(&KernelFS::mounted->KernelFS_CS);
}


char KernelFile::write(BytesCnt cnt, char* buffer) { // might need to speed it up with strcpy
	if (mode == 'r')
		return 0;
	for (BytesCnt i = 0; i < cnt; i++) {
		if (eof())
			if(!expand())
				return 0;
		data[dataCursor] = buffer[i];
		dirtyData = 1;
		seek(cursor + 1);
	}
	return 1;
}

BytesCnt KernelFile::read(BytesCnt cnt, char* buffer) { // might need to speed it up with strcpy
	for (BytesCnt i = 0; i < cnt; i++) {
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

	if (!cursorLoaded || a != ind1Cursor) {
		if (cursorLoaded) {
			if (dirtyData) {
				EnterCriticalSection(&KernelFS::mounted->KernelFS_CS);
				KernelFS::mounted->cache->writeCluster(ind2[ind2Cursor], (char*)data);
				LeaveCriticalSection(&KernelFS::mounted->KernelFS_CS);
			}
			if (dirtyInd2) {
				EnterCriticalSection(&KernelFS::mounted->KernelFS_CS);
				KernelFS::mounted->cache->writeCluster(ind1[ind1Cursor], (char*)ind2);
				LeaveCriticalSection(&KernelFS::mounted->KernelFS_CS);
			}
		}
		if (ind1[a] && ind2[b]) {
			EnterCriticalSection(&KernelFS::mounted->KernelFS_CS);
			KernelFS::mounted->cache->readCluster(ind1[a], (char*)ind2);
			KernelFS::mounted->cache->readCluster(ind2[b], (char*)data);
			LeaveCriticalSection(&KernelFS::mounted->KernelFS_CS);
			cursorLoaded = 1;
		}
		else {
			cursorLoaded = 0;
		}
		dirtyData = 0;
		dirtyInd2 = 0;
	}
	else if (b != ind2Cursor) {
		if (dirtyData) {
			EnterCriticalSection(&KernelFS::mounted->KernelFS_CS);
			KernelFS::mounted->cache->writeCluster(ind2[ind2Cursor], (char*)data);
			LeaveCriticalSection(&KernelFS::mounted->KernelFS_CS);
		}
		if (ind2[b]) {
			EnterCriticalSection(&KernelFS::mounted->KernelFS_CS);
			KernelFS::mounted->cache->readCluster(ind2[b], (char*)data);
			LeaveCriticalSection(&KernelFS::mounted->KernelFS_CS);
		}
		else
			cursorLoaded = 0;
		dirtyData = 0;
	}

	ind1Cursor = a;
	ind2Cursor = b;
	dataCursor = c;
	cursor = r;
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
	if (mode == 'r')
		return 0;
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
		dirtyInd2 = 1;
		ind2[b] = 0;
		b++;
	}


	ClusterNo tempInd2[entries];
	a += 1;
	while (a < entries && ind1[a] != (ClusterNo)0) {
		b = 0;
		EnterCriticalSection(&KernelFS::mounted->KernelFS_CS);
		KernelFS::mounted->cache->readCluster(ind1[a], (char*)tempInd2);
		LeaveCriticalSection(&KernelFS::mounted->KernelFS_CS);
		while (b < entries && tempInd2[b] != (ClusterNo)0) {
			KernelFS::mounted->dealloc(tempInd2[b]);
			b++;
		}
		KernelFS::mounted->dealloc(ind1[a]);
		dirtyInd1 = 1;
		ind1[a] = 0;
		a++;
	}
	dirtyData = 1;
	if (cursor == 0) {
		KernelFS::mounted->dealloc(ind2[0]);
		KernelFS::mounted->dealloc(ind1[0]);
		ind1[0] = 0;
		dirtyInd2 = 0;
		dirtyData = 0;
		dirtyInd1 = 1;
		cursorLoaded = 0;
	}
	sizeOfFile = cursor;
	return 1;
}

char KernelFile::expand()
{
	int r = cursor;
	int A = ClusterSize / sizeof(ClusterNo);
	int B = A;
	int C = ClusterSize;
	int a = r / (B*C); // ind1Pointer
	int b = (r - a * B*C) / C; //ind2Pointer
	int c = r - a * B*C - b * C; //dataBytePointer
	//if (cursorLoaded)
		//exit(333); //check
	if (c == 0) {
		if (b == 0) {
			ClusterNo newInd2 = KernelFS::mounted->alloc();
			ind1[a] = newInd2;
			dirtyInd1 = 1;
			EnterCriticalSection(&KernelFS::mounted->KernelFS_CS);
			KernelFS::mounted->cache->readCluster(ind1[a], (char*)ind2);
			LeaveCriticalSection(&KernelFS::mounted->KernelFS_CS);
			dirtyInd2 = 0;
		}
		ClusterNo newData = KernelFS::mounted->alloc();
		ind2[b] = newData;
		EnterCriticalSection(&KernelFS::mounted->KernelFS_CS);
		KernelFS::mounted->cache->readCluster(ind2[b], (char*)data);
		LeaveCriticalSection(&KernelFS::mounted->KernelFS_CS);
		cursorLoaded = 1;
		dirtyInd2 = 1;
		dirtyData = 0;
	}
	sizeOfFile++;
	return 1;
}