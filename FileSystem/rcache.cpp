#include "rcache.h"

int RealCache::choose()
{
	while (clocks[hand] > 0) {
		clocks[hand]--;
		hand = (hand + 1) % size;
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
	int hand = 0;
}

ClusterNo RealCache::getCacheNumOfClusters() const
{
	return size;
}

int RealCache::sync()
{
	for (unsigned i = 0; i < size; i++) {
		if(clocks[i] == 0 && dirty[i])
			p->writeCluster(adrs[i], cache + ClusterSize * i);
	}
	return 1;
}

int RealCache::readCluster(ClusterNo adr, char * buffer)
{
	unsigned i;
	for (i = 0; i < size; i++)
	{
		if (adrs[i] == adr)
			break;
	}
	if (i == size) {
		i = choose();
		if (clocks[i] == 0 && dirty[i]) {
			p->writeCluster(adrs[i], cache + i * ClusterSize);
		}
		else
			clocks[i] = 0;
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
		if (adrs[i] == adr)
			break;
	}
	if (i == size) {
		i = choose();
		if (clocks[i] == 0 && dirty[i]) {
			p->writeCluster(adrs[i], cache + i * ClusterSize);
		}
		else
			clocks[i] = 0;
		adrs[i] = adr;
		dirty[i] = 0;
		p->readCluster(adrs[i], cache + i * ClusterSize);
	}

	memcpy(cache + ClusterSize * i, buffer, ClusterSize);
	dirty[i] = 1;
	if (clocks[i] < clockLimit)
		clocks[i]++;
	return 1;
}

RealCache::~RealCache()
{
	delete[] clocks;
	delete[] dirty;
	delete[] cache;
	delete[] adrs;
}


