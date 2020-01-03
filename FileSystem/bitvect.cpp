#include "bitvect.h"
#include "cache.h"
#include <stdlib.h>
#include <iostream>

BitVector::BitVector(Cache * c)
{
	this->c = c;
	c->readCluster(0, bits);
}

void BitVector::set(ClusterNo cNo)
{
	int numOfByte = cNo / 8;
	int numOfBit = cNo % 8;
	char mask = 0x01 << (7 - numOfBit);
	bits[numOfByte] |= mask;
}

void BitVector::reset(ClusterNo cNo)
{
	int numOfByte = cNo / 8;
	int numOfBit = cNo % 8;
	char mask = ~(0x01 << (7 - numOfBit));
	bits[numOfByte] &= mask;
}

int BitVector::get(ClusterNo cNo)
{
	ClusterNo numOfByte = cNo / 8UL;
	int numOfBit = cNo % 8;
	char mask = 0x01 << (7 - numOfBit);
	return bits[numOfByte] & mask;
}

void BitVector::writeThrough()
{
	c->writeCluster(0, bits);
}

void BitVector::clear()
{
	memset(bits, 0, ClusterSize);
}

ClusterNo BitVector::find()
{
	char freeByte = 0;
	ClusterNo freeCluster = 0;
	for (ClusterNo i = 0; i < ClusterSize; i++) {
		if (bits[i] != 0xFF) {
			freeByte = bits[i];
			freeCluster = i * 8;
			break;
		}
	}
	if (freeByte == 0)
		return 0;

	for (int j = 0; j < 8; j++) {
		char mask = 0x01 << (7 - j);
		if (!(bits[freeByte] & mask)) {
			return freeCluster + j;
		}
	}
}

BitVector::~BitVector()
{
}
