#include "bitvect.h"
#include "cache.h"
#include <stdlib.h>
#include <iostream>

BitVector::BitVector(Cache * c) : BitVector(c, 0, c->getNumOfClusters())
{
}

BitVector::BitVector(Cache* c, ClusterNo id, ClusterNo size) {
	this->c = c;
	c->readCluster(bitVectorId, bits);
	bitVectorId = id;
	if (size > ClusterSize)
		next = new BitVector(c, bitVectorId + 1, size - ClusterSize);
	else
		next = NULL;
}

void BitVector::set(ClusterNo cNo)
{
	if (cNo >= ClusterSize)
		return next->set(cNo - ClusterSize);
	int numOfByte = cNo / 8;
	int numOfBit = cNo % 8;
	char mask = 0x01 << (7 - numOfBit);
	bits[numOfByte] |= mask;
}

void BitVector::reset(ClusterNo cNo)
{
	if (cNo >= ClusterSize)
		return next->reset(cNo - ClusterSize);
	int numOfByte = cNo / 8;
	int numOfBit = cNo % 8;
	char mask = ~(0x01 << (7 - numOfBit));
	bits[numOfByte] &= mask;
}

int BitVector::get(ClusterNo cNo)
{
	if (cNo >= ClusterSize)
		return next->get(cNo - ClusterSize);
	ClusterNo numOfByte = cNo / 8UL;
	int numOfBit = cNo % 8;
	char mask = 0x01 << (7 - numOfBit);
	return bits[numOfByte] & mask;
}

void BitVector::writeThrough()
{
	c->writeCluster(bitVectorId, bits);
	if (next)
		next->writeThrough();
}

void BitVector::clear()
{
	memset(bits, 0, ClusterSize);
	if (next)
		next->clear();
}

ClusterNo BitVector::find() // TODO
{
	char freeByte = 0;
	ClusterNo freeCluster = 0;
	ClusterNo i;
	for (i = 0; i < ClusterSize; i++) {
		if (bits[i] != 0xFF) { // index and bitvector clusters must be set to 1 for this to work
			freeByte = bits[i];
			freeCluster = i * 8;
			break;
		}
	}
	if (i == ClusterSize) {
		if(next)
			return next->find();
		return 0;
	}

	for (int j = 0; j < 8; j++) {
		char mask = 0x01U << (7 - j);
		if (!(bits[freeByte] & mask)) {
			return freeCluster + j;
		}
	}
	return 0;
}

BitVector::~BitVector()
{
	writeThrough();
	delete next;
}
