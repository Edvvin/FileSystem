#include "kfs.h"



KernelFS::KernelFS() {

}

KernelFS::~KernelFS() {

}

char KernelFS::mount(Partition* partition, KernelFS*& kfs) {

}
char KernelFS::unmount(KernelFS*& kfs) {

}

char KernelFS::format() {

}
FileCnt KernelFS::readRootDir() {

}
char KernelFS::doesExist(char* fname) {

}

File* KernelFS::open(char* fname, char mode) {

}
char KernelFS::deleteFile(char* fname) {

}

