#include "cache.h"


Cache::Cache(Partition * p)
{
	this->p = p;
}

ClusterNo Cache::getCacheNumOfClusters() {
	return p->getNumOfClusters();
}

void Cache::writeBack() // TODO
{
	return;
}

int Cache::readCluster(ClusterNo cNo, char * buffer)
{
	return p->readCluster(cNo, buffer);
}

int Cache::writeCluster(ClusterNo cNo, const char * buffer)
{
	return p->writeCluster(cNo, buffer);
}

void Cache::clear(int doWriteBack)
{
}

Cache::~Cache()
{
}
