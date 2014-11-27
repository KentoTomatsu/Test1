#pragma once
#include<windows.h>
class Measure
{
public:
	Measure();
	~Measure();
	void start();	//���Ԍv���J�n
	void end();	//���Ԍv���I��
	void showMemory();	//�������g�p�󋵊m�F
	double getFreq();
	double getOverHead();
	double getDiff();
private:
	LARGE_INTEGER Freq,Time1,Time2,OverHead,Diff;	//���Ԍv���p�ϐ�
	bool underCounting;
};

