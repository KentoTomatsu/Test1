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
	//���g�����擾
	ZeroMemory(&Freq, sizeof(LARGE_INTEGER));
	QueryPerformanceFrequency(&Freq);
	//�֐��Ăяo���I�[�o�[�w�b�h�̑���
	QueryPerformanceCounter(&Time1);
	QueryPerformanceCounter(&Time2);
	OverHead.QuadPart = Time2.QuadPart-Time1.QuadPart;
	QueryPerformanceCounter(&Time1);	//�v���J�n
}
void Measure::end(void){
	QueryPerformanceCounter(&Time2);	//�v���I��
	Diff.QuadPart = Time2.QuadPart - Time1.QuadPart - OverHead.QuadPart;	//�v�����Ԃ̌v�Z
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