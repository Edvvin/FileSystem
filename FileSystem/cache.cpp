#include "cache.h"


Cache::Cache(Partition * p)
{
	this->p = p;
}

ClusterNo Cache::getNumOfClusters() const
{
	return p->getNumOfClusters();
}

int Cache::readCluster(ClusterNo cNo, char * buffer)
{
	return p->readCluster(cNo, buffer);
}

int Cache::writeCluster(ClusterNo cNo, const char * buffer)
{
	return p->writeCluster(cNo, buffer);
}

Cache::~Cache()
{
}
