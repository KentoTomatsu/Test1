#pragma once
#include "GA.h"

const int MAX_KEY=127;
const int MAX_VELOCITY=127;
const double EXECUTE_MUTATION=1/BASE_NUM;
const double SELF_COPY_RATE=0.5;
const int MAX_SERIAL=4;

class GAForMusic : public GA
{
public:
	GAForMusic(void);
	~GAForMusic(void);
	virtual void setGA();
	virtual void startGA();
	virtual void initializeParameters(void);	//GAのパラメータの初期化
	virtual void initializeGenes(void);	//染色体の初期化
	virtual void printParameters(void);	//GAのパラメータの表示
	void setBase(int ***base,int *musicLength,int length,int scale);
	double getSelfCopyRate();
	double *getFitnessWeight();
	double getLocalSimilarity();
protected:
	int scale;
	//KEY tonic;
	int medianTone;
	double selfCopyRate;
	int measureLength;
	double localSimilarity;

	void executeProcessing();
	virtual void executeCrossover(void);
	virtual void executeMutation();
	void multiPointCrossover(int n1,int n2);
	void selfCopy();
	void setMedianTone();
	void modifyMelodyLine();
	void modifyDuration();
	bool meetTheory();
};