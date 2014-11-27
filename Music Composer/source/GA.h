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
	virtual void initializeParameters(void);	//GA�̃p�����[�^�̏�����	
	virtual void initializeGenes(void);	//���F�̂̏�����
	virtual void printParameters(void);	//GA�̃p�����[�^�̕\��	
	void allocateGA(void);	//GA�Ŏg���z����m�ۂ���	
	void deleteGA(void);	//GA�Ŏg���Ĕz����J������
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
	int ***genotype;	//�̂̈�`�q�^:key,vel,rise
	int **individual;
	double *fitness;	//�̂̓K���x
	double **detailFitness;
	double **detailMusicTheory;
	double **detailGlobalSimilarity;
	int ***newGenotype;	//�V�̂̈�`�q��
	double *newFitness;	//�V�̂̓K���x
	double **newDetailFitness;
	int ***baseGenotype;
	int baseLength;
	int *baseMusicLength;
	int pattern[SCALE_LENGTH];
	int  **eliteGenotype;	//�G���[�g�̈�`�q��
	double eliteFitness;	//�G���[�g�̓K���x
	double eliteDetailFitness[SUPERORDINATE_FITNESS_LENGTH];
	double eliteDetailMusicTheory[FITNESS_LENGTH];
	double eliteDetailGlobalSimilarity[GLOBAL_SIMILARITY_LENGTH];
	int eliteNumber;	//�G���[�g�̌̔ԍ�
	int eliteGeneration;
	int *number;	//�����Ŏg���z��
	double *rouletteTable;	//���[���b�g
	int popSize;	//�̑���	
	int geneSize;	//���F�̒�
	int generation;
	double crossoverRate;	//������(0-1)
	double mutationRate;	//�ˑR�ψٗ�(0-1)
	int eliteFlag;	//�G���[�g�ۑ��̗L��(1����C0�Ȃ�)
	int crossType;	//�������@(1�_�C2�_�C���߂���, ��l)
	bool executeLS;
	double thresholdExecuteLocalSim;
	bool updateElite;
	Theorist theorist;
	MyRandom random;
	FileManager fileManager;

	void swapUnsignedChar(unsigned char *n1,unsigned char *n2);	//2��unsigned char������	
	void swapInt(int *n1,int *n2);	//2��int������
	void copyNewToOld(void);	//�z��̃R�s�[
	void onePointCrossover(int n1,int n2);	//1�_����
	void twoPointCrossover(int n1,int n2);	//2�_����
	void uniformCrossover(int n1,int n2);	//��l����
	virtual void executeCrossover();	//���ׂĂ̌̂���x���I�΂��悤�Ɍ���������
	virtual void executeMutation();	//�S�̂̑S��`�q�̓ˑR�ψ�
	void selectionUsingRouletteRule(void);	//���[���b�g�I�����s��
	void findAndSetBestIndividual();	//�ŗǌ̂�T��
	void elitistStrategy(void);	//�G���[�g�ۑ��헪���s��
	void searchSheet();
	void printBestIndividual(int generation);	//�œK�̂�\������
	void printBaseIndividual();
	void printBaseFitness();
	void printAverageDetailFitness();
	void printSheet();
	bool checkLocalSimilarity();
};