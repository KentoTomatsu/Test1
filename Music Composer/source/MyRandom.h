#pragma once
#include"mt19937ar.h"

const unsigned int GENRAND_MAX=0xffffffff;

class MyRandom
{
public:
	MyRandom(void);
	~MyRandom(void);
	int nextInt(int max);
	double nextDouble(double max);
private:
	void initializeRandomNumber(void);
};

