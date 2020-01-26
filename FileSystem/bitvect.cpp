#include "bitvect.h"
#include "cache.h"
#include <stdlib.h>
#include <iostream>

BitVector::BitVector(Partition* p) 
	: BitVector(p, 0, ((p->getNumOfClusters() - 1) / (ClusterSize*CHAR_BIT)) + 1) // calculate the number of clusters needed
{
}

BitVector::BitVector(Partition* p, ClusterNo id, ClusterNo size) {
	this->p = p;
	clusterCnt = size;
	bitVectorId = id;
	p->readCluster(bitVectorId, bits);
	dirty = 0;
	if (size > 1)
		next = new BitVector(p, bitVectorId + 1, size - 1);
	else
		next = NULL;
}

void BitVector::set(ClusterNo cNo)
{
	if (cNo >= ClusterSize*CHAR_BIT)
		return next->set(cNo - ClusterSize * CHAR_BIT);
	ClusterNo numOfByte = cNo / CHAR_BIT;
	ClusterNo numOfBit = cNo % CHAR_BIT;
	char mask = 0x01U << (CHAR_BIT - 1 - numOfBit);
	bits[numOfByte] |= mask;
	dirty = 1;
}

void BitVector::reset(ClusterNo cNo)
{
	if (cNo >= ClusterSize * CHAR_BIT)
		return next->reset(cNo - ClusterSize * CHAR_BIT);
	ClusterNo numOfByte = cNo / CHAR_BIT;
	ClusterNo numOfBit = cNo % CHAR_BIT;
	char mask = ~(0x01U << (CHAR_BIT - 1 - numOfBit));
	bits[numOfByte] &= mask;
	dirty = 1;
}

int BitVector::get(ClusterNo cNo)
{
	if (cNo >= ClusterSize)
		return next->get(cNo - ClusterSize);
	ClusterNo numOfByte = cNo / CHAR_BIT;
	int numOfBit = cNo % CHAR_BIT;
	char mask = 0x01U << (CHAR_BIT - 1 - numOfBit);
	return bits[numOfByte] & mask;
}

void BitVector::clear()
{
	memset(bits, 0, ClusterSize);
	if (next)
		next->clear();
	dirty = 1;
}

ClusterNo BitVector::size()
{
	return this->clusterCnt;
}

ClusterNo BitVector::find()
{
	char freeByte = 0;
	ClusterNo freeCluster = 0;
	ClusterNo i;
	for (i = 0; i < ClusterSize; i++) {
		if (bits[i] != 0xFF) { // index and bitvector clusters must be set to 1 for this to work
			freeByte = bits[i];
			freeCluster = i * CHAR_BIT;
			break;
		}
	}
	if (i == ClusterSize) {
		if(next)
			return next->find();
		return 0;
	}

	for (int j = 0; j < CHAR_BIT; j++) {
		char mask = 0x01U << (CHAR_BIT - 1 - j);
		if (!(bits[freeByte] & mask)) {
			return freeCluster + j;
		}
	}
	exit(420); //check
	return 0;
}

BitVector::~BitVector()
{
	if(dirty)
		p->writeCluster(bitVectorId, bits);
	delete next;
}
