#pragma once
#include"common.h"
#include"Theorist.h"
#include"FileManager.h"

const int MAX_GENERATION=1000;
const int POP_SIZE=300;
const int ONE_POINT=0;
const int TWO_POINT=1;
const int MULTI_POINT=2;
const int UNIFORM=3;
const double CROSSOVER_RATE=0.8;
const double MUTATION_RATE=0.1;
const int EXECUTE_LS_SPAN=1;
const int EXECUTE_FINAL_LS=500;
const int PRINT_SPAN=1000;

class GA
{
public:
	GA(void);
	~GA(void);
	virtual void setGA();
	virtual void startGA();
	virtual void initializeParameters(void);	//GAのパラメータの初期化	
	virtual void initializeGenes(void);	//染色体の初期化
	virtual void printParameters(void);	//GAのパラメータの表示	
	void allocateGA(void);	//GAで使う配列を確保する	
	void deleteGA(void);	//GAで使って配列を開放する
	void calcFitness();
	void calcBaseFitness();
	int **getBestIndividual();
	int getPopSize();
	int getMusicLength();
	int getMaxGeneration();
	double getCrossoverRate();
	double getMutationRate();
	int getEliteGeneration();
	double *getEliteDetailFitness();
	double *getEliteDetailMusicTheory();
	double *getEliteDetailGlobalSimilarity();
	double getPitchRange();
	double *getImpulseRate();
	double getAverageImpulseRate();
	double getOverAverageImpulseRate();
protected:
	int ***genotype;	//個体の遺伝子型:key,vel,rise
	int **individual;
	double *fitness;	//個体の適応度
	double **detailFitness;
	double **detailMusicTheory;
	double **detailGlobalSimilarity;
	int ***newGenotype;	//新個体の遺伝子座
	double *newFitness;	//新個体の適応度
	double **newDetailFitness;
	int ***baseGenotype;
	int baseLength;
	int *baseMusicLength;
	int pattern[SCALE_LENGTH];
	int  **eliteGenotype;	//エリートの遺伝子座
	double eliteFitness;	//エリートの適応度
	double eliteDetailFitness[SUPERORDINATE_FITNESS_LENGTH];
	double eliteDetailMusicTheory[FITNESS_LENGTH];
	double eliteDetailGlobalSimilarity[GLOBAL_SIMILARITY_LENGTH];
	int eliteNumber;	//エリートの個体番号
	int eliteGeneration;
	int *number;	//交叉で使う配列
	double *rouletteTable;	//ルーレット
	int popSize;	//個体総数	
	int geneSize;	//染色体長
	int generation;
	double crossoverRate;	//交叉率(0-1)
	double mutationRate;	//突然変異率(0-1)
	int eliteFlag;	//エリート保存の有無(1あり，0なし)
	int crossType;	//交叉方法(1点，2点，小節ごと, 一様)
	bool executeLS;
	double thresholdExecuteLocalSim;
	bool updateElite;
	Theorist theorist;
	MyRandom random;
	FileManager fileManager;

	void swapUnsignedChar(unsigned char *n1,unsigned char *n2);	//2つのunsigned charを交換	
	void swapInt(int *n1,int *n2);	//2つのintを交換
	void copyNewToOld(void);	//配列のコピー
	void onePointCrossover(int n1,int n2);	//1点交叉
	void twoPointCrossover(int n1,int n2);	//2点交叉
	void uniformCrossover(int n1,int n2);	//一様交叉
	virtual void executeCrossover();	//すべての個体が一度ずつ選ばれるように交叉させる
	virtual void executeMutation();	//全個体の全遺伝子の突然変異
	void selectionUsingRouletteRule(void);	//ルーレット選択を行う
	void findAndSetBestIndividual();	//最良個体を探す
	void elitistStrategy(void);	//エリート保存戦略を行う
	void searchSheet();
	void printBestIndividual(int generation);	//最適個体を表示する
	void printBaseIndividual();
	void printBaseFitness();
	void printAverageDetailFitness();
	void printSheet();
	bool checkLocalSimilarity();
};