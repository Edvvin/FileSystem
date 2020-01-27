#include "rcache.h"

int RealCache::choose()
{
	while (clocks[hand] > 0) {
		if(remaining == 0)
			clocks[hand]--;
		hand = (hand + 1) % size;
	}
	if (clocks[hand] < 0) {
		remaining--;
		if (remaining < 0)
			exit(5555); //check
		clocks[hand] = 0;
	}
	return hand;
}

RealCache::RealCache(Partition * p, ClusterNo size) : Cache(p)
{
	this->size = size;
	clocks = new int[size];
	dirty = new char[size];
	for (unsigned i = 0; i < size; i++) {
		clocks[i] = -1;
		dirty[i] = 0;
	}
	cache = new char[ClusterSize*size];
	adrs = new ClusterNo[size];
	remaining = size;
	int hand = 0;
}

ClusterNo RealCache::getCacheNumOfClusters()
{
	return size;
}

void RealCache::writeBack()
{
	for (unsigned i = 0; i < size; i++) {
		if(dirty[i])
			p->writeCluster(adrs[i], cache + ClusterSize * i);
	}
	return;
}

int RealCache::readCluster(ClusterNo adr, char * buffer)
{
	ClusterNo i;
	for (i = 0; i < size; i++)
	{
		if (clocks[i] >= 0 && adrs[i] == adr)
			break;
	}
	if (i == size) {
		i = choose();
		if (dirty[i]) {
			p->writeCluster(adrs[i], cache + i * ClusterSize);
		}
		adrs[i] = adr;
		dirty[i] = 0;
		p->readCluster(adrs[i], cache + i * ClusterSize);
	}
	memcpy(buffer, cache + ClusterSize * i, ClusterSize);
	if(clocks[i] < clockLimit)
		clocks[i]++;
	return 1;
}

int RealCache::writeCluster(ClusterNo adr, const char * buffer)
{
	unsigned i;
	for (i = 0; i < size; i++)
	{
		if (clocks[i] >= 0 && adrs[i] == adr)
			break;
	}
	if (i == size) {
		i = choose();
		if (dirty[i]) {
			p->writeCluster(adrs[i], cache + i * ClusterSize);
		}
		adrs[i] = adr;
	}
	memcpy(cache + ClusterSize * i, buffer, ClusterSize);
	dirty[i] = 1;
	if (clocks[i] < clockLimit)
		clocks[i]++;
	return 1;
}

void RealCache::clear(int doWriteBack)
{
	if (doWriteBack)
		writeBack();
	for (unsigned i = 0; i < size; i++)
	{
		dirty[i] = 0;
		clocks[i] = -1;
	}
	remaining = size;
}

RealCache::~RealCache()
{
	writeBack();
	delete[] clocks;
	delete[] dirty;
	delete[] cache;
	delete[] adrs;
}


