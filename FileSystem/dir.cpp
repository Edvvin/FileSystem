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

char Directory::getDirDesc(char * fileName, DirDesc* desc, int& i)
{
	if (*fileName != '/')
		return 0;
	fileName++;
	kf->seek(0);
	i = 0;
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
		i++;
	}
	return 0;
}

char Directory::addDirDesc(DirDesc * desc)
{
	kf->seek(0);
	DirDesc dd;
	while (!kf->eof()) {
		kf->read(sizeof(DirDesc), (char*)&dd);
		if (!strncmp(desc->name, "\0\0\0\0\0\0\0\0", FNAMELEN)) {
			kf->seek(kf->filePos() - sizeof(DirDesc));
			break;
		}
	}
	kf->write(sizeof(DirDesc), (char*)desc);
}

char Directory::clearDirDesc(int i)
{
	kf->seek(i * sizeof(DirDesc));
	DirDesc dd;
	kf->read(sizeof(DirDesc), (char*)&dd);
	memset(&(dd.name), 0, 8 * sizeof(char));
	kf->write(sizeof(DirDesc), (char*)&dd);
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