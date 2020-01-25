#pragma once
#include "part.h"

//I might implement this
class Cache
{
protected:
	Partition* p;
public:
	
	ClusterNo getNumOfClusters() const;
	virtual ClusterNo getCacheNumOfClusters() const;
	virtual int sync();
	virtual int readCluster(ClusterNo, char *buffer);
	virtual int writeCluster(ClusterNo, const char *buffer);

	virtual ~Cache();
	Cache(Partition* p);
};

