#include "dir.h"
#include "kfile.h"
#include "cache.h"

Directory::Directory(KernelFS * fs)
{
	kf = new KernelFile(fs->cache->getNumOfClusters()/sizeof(ClusterNo), 'w');
}

Directory::~Directory()
{
	delete kf;
}

char Directory::getDirDesc(char * fileName, DirDesc* desc)
{
	if (*fileName != '/')
		return 0;
	fileName++;
	kf->seek(0);
	while (!kf->eof()) {
		kf->read(sizeof(DirDesc), (char*)desc);
		if (!strncmp(desc->name, fileName, FNAMELEN)) {
			fileName = strchr(fileName, '.');
			if (!fileName)
				return 0;
			if (!strncmp(desc->ext, fileName, FEXTLEN)) {
				return 1;
			}
		}
	}
	return 0;
}

FileCnt Directory::cntFiles() {
	FileCnt cnt = 0;
	kf->seek(0);
	DirDesc dd;
	while (!kf->eof()) {
		kf->read(sizeof(DirDesc), (char*)&dd);
		cnt++;
	}
	return cnt;
}