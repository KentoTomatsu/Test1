#include "MyRandom.h"
#include <ctime>

MyRandom::MyRandom(void)
{
	initializeRandomNumber();
}
MyRandom::~MyRandom(void)
{
}
void MyRandom::initializeRandomNumber(void){
	init_genrand((unsigned long)time(NULL));
}
int MyRandom::nextInt(int max){
	return (int)((double)genrand_int32()/((double)GENRAND_MAX+1.0)*max);
}
double MyRandom::nextDouble(double max){
	return (double)genrand_int32()/((double)GENRAND_MAX+1.0)*max;
}
