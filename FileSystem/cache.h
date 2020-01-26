#pragma once
#include "part.h"

//I might implement this
class Cache
{
protected:
	Partition* p;
public:
	
	virtual ClusterNo getCacheNumOfClusters();
	virtual void writeBack();
	virtual int readCluster(ClusterNo, char *buffer);
	virtual int writeCluster(ClusterNo, const char *buffer);
	virtual void clear(int doWriteBack);
	virtual ~Cache();
	Cache(Partition* p);
};

