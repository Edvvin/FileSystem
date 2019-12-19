#include "file.h"
#include "kfile.h"


File::~File() {
	//TODO
}

char File::write(BytesCnt cnt, char* buffer) {
	myImpl->write(cnt, buffer);
}
BytesCnt File::read(BytesCnt cnt, char* buffer) {
	myImpl->read(cnt, buffer);
}
char File::seek(BytesCnt cnt) {
	myImpl->seek(cnt);
}
BytesCnt File::filePos() {
	myImpl->filePos();
}
char File::eof() {
	myImpl->eof();
}
BytesCnt File::getFileSize() {
	myImpl->getFileSize();
}
char File::truncate() {
	myImpl->truncate();
}

File::File() {
	//TODO
}