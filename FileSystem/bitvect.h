#pragma once
#include "part.h"

class Cache;
class BitVector
{
	Partition* p;
	ClusterNo bitVectorId;
	char bits[ClusterSize];
	char dirty;
	BitVector* next;
	ClusterNo clusterCnt;
public:
	BitVector(Partition* p);
	BitVector(Partition* c, ClusterNo id, ClusterNo size);
	void set(ClusterNo cNo);
	void reset(ClusterNo cNo);
	int get(ClusterNo cNo);
	void clear();
	ClusterNo size();
	ClusterNo find();
	~BitVector();
};

