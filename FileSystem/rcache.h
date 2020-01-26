#pragma once
#include "kfs.h"
#include "kfile.h"
#include "cache.h"
#include "part.h"


class RealCache : public Cache {
	ClusterNo size;
	int* clocks;
	char* dirty;
	ClusterNo* adrs;
	char* cache;
	int hand;
	int remaining;
	int choose();

public:
	static const int clockLimit = 4;
	RealCache(Partition* p, ClusterNo size);

	ClusterNo getCacheNumOfClusters() override;
	void writeBack() override;
	int readCluster(ClusterNo adr, char *buffer) override;
	int writeCluster(ClusterNo adr, const char *buffer) override;
	void clear(int doWriteBack) override;
	~RealCache();
};