#pragma once
#include "part.h"

class Cache;
class BitVector
{
	Cache* c;
	ClusterNo bitVectorId;
	char bits[ClusterSize];
	BitVector* next;
public:
	BitVector(Cache* c);
	BitVector(Cache* c, ClusterNo id, ClusterNo size);
	void set(ClusterNo cNo);
	void reset(ClusterNo cNo);
	int get(ClusterNo cNo);
	void writeThrough();
	void clear();
	ClusterNo find();
	~BitVector();
};

