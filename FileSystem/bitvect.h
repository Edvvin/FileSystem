#pragma once
#include "part.h"

class Cache;
class BitVector
{
	Cache* c;
	char bits[ClusterSize];
public:
	BitVector(Cache* c);
	void set(ClusterNo cNo);
	void reset(ClusterNo cNo);
	int get(ClusterNo cNo);
	void writeThrough();
	void clear();
	ClusterNo find();
	~BitVector();
};

