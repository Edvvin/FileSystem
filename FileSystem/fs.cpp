#include "fs.h"
#include <iostream>
#include "part.h"
#include "kfs.h"

KernelFS* FS::myImpl = NULL;

FS::~FS() {

}

char FS::mount(Partition* partition) {
	return KernelFS::mount(partition, myImpl);
}
char FS::unmount() {
	if (myImpl == NULL) {
		return 0;
	}
	return KernelFS::unmount(myImpl);
}
char FS::format() {
	if (myImpl == NULL) {
		return 0;
	}
	return myImpl->format();
}


FileCnt FS::readRootDir() {
	if (myImpl == NULL) {
		return 0;
	}
	return myImpl->readRootDir();
}

char FS::doesExist(char* fname) {
	if (myImpl == NULL) {
		return 0;
	}
	return myImpl->doesExist(fname);
}

File* FS::open(char* fname, char mode) {
	if (myImpl == NULL) {
		return 0;
	}
	//return myImpl->open(fname, mode);
}

char FS::deleteFile(char* fname) {
	if (myImpl == NULL) {
		return 0;
	}
	return myImpl->deleteFile(fname);
}

FS::FS() {
}

