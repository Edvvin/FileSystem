#include "fs.h"
#include <iostream>
#include "part.h"
#include "kfs.h"


KernelFS* FS::myImpl = NULL;

FS::~FS() {

}

char FS::mount(Partition* partition) {
	return KernelFS::mount(partition);
}
char FS::unmount() {
	return KernelFS::unmount();
}
char FS::format() {
	if (KernelFS::mounted == NULL) {
		return 0;
	}
	return KernelFS::mounted->format();
}


FileCnt FS::readRootDir() {
	if (KernelFS::mounted == NULL) {
		return 0;
	}
	return KernelFS::mounted->readRootDir();
}

char FS::doesExist(char* fname) {
	if (KernelFS::mounted == NULL) {
		return 0;
	}
	return KernelFS::mounted->doesExist(fname);
}

File* FS::open(char* fname, char mode) {
	if (KernelFS::mounted == NULL) {
		return 0;
	}
	return KernelFS::mounted->open(fname, mode);
}

char FS::deleteFile(char* fname) {
	if (KernelFS::mounted == NULL) {
		return 0;
	}
	return KernelFS::mounted->deleteFile(fname);
}

FS::FS() {
}

