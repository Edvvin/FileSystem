#pragma once
#include "part.h"

//I might implement this
class Cache
{
	Partition* p;
public:
	
	ClusterNo getNumOfClusters() const;
	int sync();
	int readCluster(ClusterNo, char *buffer);
	int writeCluster(ClusterNo, const char *buffer);

	~Cache();
	Cache(Partition* p);
};

