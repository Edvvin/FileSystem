#include "fs.h"
#include <iostream>
#include "part.h"


KernelFS* FS::myImpl = NULL;

FS::~FS() {

}

char FS::mount(Partition* partition) {

}
char FS::unmount() {

}
char FS::format() {
}


FileCnt FS::readRootDir() {

}

char FS::doesExist(char* fname) {

}

File* open(char* fname, char mode) {


}

char deleteFile(char* fname) {

}

FS::FS() {

}

