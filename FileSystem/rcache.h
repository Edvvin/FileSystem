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

	int choose();

public:
	static const int clockLimit = 8;
	RealCache(Partition* p, ClusterNo size);

	ClusterNo getCacheNumOfClusters() const override;
	int sync() override;
	int readCluster(ClusterNo adr, char *buffer) override;
	int writeCluster(ClusterNo adr, const char *buffer) override;

	~RealCache();
};