#pragma once
#include<windows.h>
class Measure
{
public:
	Measure();
	~Measure();
	void start();	//時間計測開始
	void end();	//時間計測終了
	void showMemory();	//メモリ使用状況確認
	double getFreq();
	double getOverHead();
	double getDiff();
private:
	LARGE_INTEGER Freq,Time1,Time2,OverHead,Diff;	//時間計測用変数
	bool underCounting;
};

