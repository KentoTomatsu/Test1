#pragma once
#include"common.h"
#include<string>
#include <sstream>

class PropertiesManager
{
public:
	PropertiesManager(void);
	~PropertiesManager(void);
	void setBaseMusicName(string *name);
	void setBaseLength(int length);
	void setGAParameters(int popSize,int geneSize,int maxGeneration,double crossoverRate,double mutationRate,double selfCopyRate);
	void setTime(double time);
	void setFitnessWeight(double *fitnessWeight);
	void setEliteGeneration(int generation);
	void setEliteDetailFitness(double *detailFitness);
	void setEliteDetailMusicTheory(double *detailMusicTheory);
	void setEliteDetailGlobalSimilarity(double *detailGlobalSimilarity);
	void setLocalSimilarity(double localSimilarity);
	void setKeyFreq();
	void setPitchRange(double range);
	void setImpulseRate(double *impulse);
	void setAverageImpulseRate(double impulse);
	void setOverAverageImpulseRate(double impulse);
	void saveProperties();
private:
	string *baseMusicName;
	int baseLength;
	int *baseMusicLength;
	double time;
	KEY tonic;
	bool scale;
	int popSize;
	int geneSize;
	double crossoverRate;
	double mutationRate;
	double selfCopyRate;
	int maxGeneration;
	int eliteGeneration;
	int eliteStrategy;
	double fitnessWeight[FITNESS_LENGTH];
	double eliteFitness;
	double eliteDetailFitness[SUPERORDINATE_FITNESS_LENGTH];
	double eliteDetailMusicTheory[FITNESS_LENGTH];
	double eliteDetailGlobalSimilarity[GLOBAL_SIMILARITY_LENGTH];
	double localSimilarity;
	int keyFreq[SCALE_LENGTH];
	double pitchRange;
	double impulseRate[BASE_NUM];
	double averageImpulseRate;
	double overAverageImpulseRate;
	string createStringData();

	template<class X> string createString(string name,X data){
		ostringstream value;
		value << name << ": " << data << endl;
		return value.str();
	}
};

