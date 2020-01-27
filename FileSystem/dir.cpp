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
		if (ind1[a] && ind2[b]) {
			KernelFS::mounted->cache->readCluster(ind1[a], (char*)ind2);
			KernelFS::mounted->cache->readCluster(ind2[b], (char*)data);
			cursorLoaded = 1;
		}
		else {
			cursorLoaded = 0;
		}
		dirtyData = 0;
		dirtyInd2 = 0;
	}
	else if (b != ind2Cursor) {
		if (dirtyData)
			KernelFS::mounted->cache->writeCluster(ind2[ind2Cursor], (char*)data);
		if (ind2[b])
			KernelFS::mounted->cache->readCluster(ind2[b], (char*)data);
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

Directory::Directory(ClusterNo ind1Adr)
{
	this->ind1Adr = ind1Adr;
	cursorLoaded = 0;
	dirtyData = 0;
	dirtyInd2 = 0;
	dirtyInd1 = 0;
	KernelFS::mounted->cache->readCluster(ind1Adr, (char*)ind1);
	seek(0);
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
	int end = eof(i);
	char* buffer = (char*)&dd;
	seek(i * sizeof(DirDesc));
	for (BytesCnt j = 0; j < sizeof(DirDesc); j++) {
		if (end)
			expand();
		data[dataCursor] = buffer[j];
		dirtyData = 1;
		seek(cursor + 1);
	}
	return;
}

int Directory::addFile(char * fname)
{
	char* dot = strchr(fname, '.');
	char name[9];
	char ext[4];
	strncpy(name, fname, dot - fname);
	name[dot - fname] = 0;
	strcpy(ext, dot + 1);
	DirDesc dd;
	strncpy(dd.name, name, FNAMELEN);
	strncpy(dd.ext, ext, FEXTLEN);
	dd.ind1 = KernelFS::mounted->alloc();
	dd.zero = 0;
	dd.size = 0;
	char zeros[FNAMELEN];
	memset(zeros, 0, FNAMELEN);
	DirDesc temp;
	int i = 0;
	while (1) {
		if (eof(i))
			break;
		temp = getDirDesc(i);
		if (!memcmp(temp.name, zeros, FNAMELEN))
			break;
		i++;
	}
	setDirDesc(i, dd);
	return i;
}

void Directory::find(char * fname, int & fileInd, int & exists)
{
	if (strnlen(fname, FNAMELEN + FEXTLEN + 2) > FNAMELEN + FEXTLEN + 1) {
		exists = -1;
		fileInd = 0;
	}
	char* dot = strchr(fname, '.');
	if (!dot) {
		exists = -1;
		fileInd = 0;
	}
	if (strnlen(dot, FEXTLEN + 2) > FEXTLEN + 1) {
		exists = -1;
		fileInd = 0;
	}
	char name[9];
	char ext[4];
	strncpy(name, fname, dot - fname);
	name[dot - fname] = 0;
	strcpy(ext, dot + 1);

	fileInd = 0;
	exists = 1;
	DirDesc dd;
	while (1) {
		if (eof(fileInd)) {
			exists = 0;
			break;
		}
		dd = getDirDesc(fileInd);
		if (!strcmp(dd.name, name) && !strcmp(dd.ext, ext)) {
			break;
		}
		fileInd++;
	}
}

char Directory::eof(int i)
{
	DirDesc zero;
	memset(&zero, 0, sizeof(DirDesc));
	DirDesc dd = getDirDesc(i);
	return !memcmp(&dd, &zero, sizeof(DirDesc));
}



char Directory::expand()
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
			KernelFS::mounted->cache->readCluster(ind1[a], (char*)ind2);
			dirtyInd2 = 0;
		}
		ClusterNo newData = KernelFS::mounted->alloc();
		ind2[b] = newData;
		KernelFS::mounted->cache->readCluster(ind2[b], (char*)data);
		cursorLoaded = 1;
		dirtyInd2 = 1;
		dirtyData = 0;
	}
	return 1;
}