#include "Measure.h"
#include<iostream>
using namespace std;

Measure::Measure(void)
{
	underCounting=true;
}
Measure::~Measure(void)
{
}
void Measure::start(void){
	underCounting=true;
	//周波数情報取得
	ZeroMemory(&Freq, sizeof(LARGE_INTEGER));
	QueryPerformanceFrequency(&Freq);
	//関数呼び出しオーバーヘッドの測定
	QueryPerformanceCounter(&Time1);
	QueryPerformanceCounter(&Time2);
	OverHead.QuadPart = Time2.QuadPart-Time1.QuadPart;
	QueryPerformanceCounter(&Time1);	//計測開始
}
void Measure::end(void){
	QueryPerformanceCounter(&Time2);	//計測終了
	Diff.QuadPart = Time2.QuadPart - Time1.QuadPart - OverHead.QuadPart;	//計測時間の計算
	cout << "---Result of A Measurement---" << endl;
	cout << "Freq:     " << (double)Freq.QuadPart/1000000 << "[MHz]" << endl;
	cout << "OverHead: " <<(double)OverHead.QuadPart/Freq.QuadPart*1000000 << "[usec]" << endl;
	cout << "Time:     " << (double)Diff.QuadPart/Freq.QuadPart << "[sec]" << endl;
	underCounting=false;
}
void Measure::showMemory(void){
	MEMORYSTATUSEX ms = {sizeof(MEMORYSTATUSEX)};
	GlobalMemoryStatusEx(&ms);
	DWORDLONG dwFree = ms.ullAvailPhys;
	DWORDLONG dwUsed = ms.ullTotalPhys - ms.ullAvailPhys;
	DWORDLONG dwSize = ms.ullTotalPhys;
	cout << "---Memory Utilization---" << endl;
	cout << "Size: " << dwSize << endl;
	cout << "Free: " << dwFree << endl;
	cout << "Used: " << dwUsed << endl;
}
double Measure::getFreq(){
	if(!underCounting){
		return (double)Freq.QuadPart/1000000;
	}
	return -1;
}
double Measure::getOverHead(){
	if(!underCounting){
		return (double)OverHead.QuadPart/Freq.QuadPart*1000000;
	}
	return -1;
}
double Measure::getDiff(){
	if(!underCounting){
		return (double)Diff.QuadPart/Freq.QuadPart;
	}
	return -1;
}