#include <cmath>
#include "GA.h"
#include "Measure.h"

GA::GA(void){
	eliteNumber=-1;	//�G���[�g�̌̔ԍ�
	popSize=0;	//�̑���
	geneSize=0;	//���F�̒�
	generation=0;
	crossoverRate=0.0;	//������[0,1]
	mutationRate=0.0;	//�ˑR�ψٗ�[0,1]	
	eliteFlag=FALSE;	//�G���[�g�ۑ��̗L��(1����C0�Ȃ�)	
	crossType=ONE_POINT;
	executeLS=true;
	updateElite=true;
	thresholdExecuteLocalSim=theorist.getSuperordinateFitnessWeight(LOCAL)*0.8;
	searchSheet();
}
GA::~GA(void){}
void GA::searchSheet(){
	fileManager.openStream(FileManager::getFileName(FITNESS_FILE).c_str());
}
//2��unsigned char������
void GA::swapUnsignedChar(unsigned char *n1,unsigned char *n2){
	unsigned char n;
	n=*n1;
	*n1=*n2;
	*n2=n;
}
//2��int������
void GA::swapInt(int *n1,int *n2){
	int n;
	n=*n1;
	*n1=*n2;
	*n2=n;
}
//�z��̃R�s�[
//(�V���� newGenotype => genotype, newFitness => fitness)
void GA::copyNewToOld(){
	for(int i=0;i<popSize;i++){
		for(int j=0;j<geneSize;j++){
			for(int k=0;k<NOTE_CONTENTS_LENGTH;k++){
				genotype[i][j][k] = newGenotype[i][j][k];
			}
		}
		fitness[i] = newFitness[i];
	}
}
//1�_����
//�� No.n1 �ƌ� No.n2 �̐��F�̂��P�_���������C���ƒu��������
void GA::onePointCrossover(int n1,int n2){
	int crsPnt;	//�����ʒu
	crsPnt = random.nextInt(geneSize);
	for(int i=crsPnt+1;i<geneSize;i++){
		for(int j=0;j<NOTE_CONTENTS_LENGTH;j++){
			swapInt(&genotype[n1][i][j], &genotype[n2][i][j]);
		}
	}
}
//2�_����
//�� No.n1 �ƌ� No.n2 �̐��F�̂��Q�_���������C���ƒu��������
void GA::twoPointCrossover(int n1,int n2){
	int crsPnt1,crsPnt2;	//�����_
	
	crsPnt1 = random.nextInt(geneSize);
	do{
		crsPnt2 = random.nextInt(geneSize);
	}while(crsPnt1 == crsPnt2);
	if(crsPnt1>crsPnt2){
		swapInt(&crsPnt1,&crsPnt2);
	}
	for(int i=crsPnt1+1;i<crsPnt2;i++){
		for(int j=0;j<NOTE_CONTENTS_LENGTH;j++){
			swapInt(&genotype[n1][i][j],&genotype[n2][i][j]);
		}
	}
}
//��l����
//�� No.n1 �ƌ� No.n2 �̐��F�̂���l���������C���ƒu��������
void GA::uniformCrossover(int n1,int n2){
	for(int i=0;i<geneSize;i++){
		if(random.nextInt(2)){
			for(int j=0;j<NOTE_CONTENTS_LENGTH;j++){
				swapInt(&genotype[n1][i][j],&genotype[n2][i][j]);
			}
		}
	}
}
void GA::executeCrossover(void){}
void GA::executeMutation(void){}
//���[���b�g�I�����s��
void GA::selectionUsingRouletteRule(void){
	int i,j,num;
	double sum,randReal;

	//���[���b�g�̍쐬
	//fitness�̍��v���o��
	sum=0.0;
	for(i=0;i<popSize;i++){
		sum+=(FITNESS_LENGTH+fitness[i]);
	}	
	for(i=0;i<popSize;i++){
		rouletteTable[i]=(FITNESS_LENGTH+fitness[i])/sum;
	}
	sum=0.0;

	for(i=0;i<popSize;i++){
		sum+=rouletteTable[i];
		rouletteTable[i]=sum;
	}
	//���[���b�g����
	for(i=0;i<popSize;i++){	
		randReal=random.nextDouble(1.0);	//���I�ԍ������߂�[0.0,1.0)		
		num=0;
		while(randReal>rouletteTable[num]){
			num++;
		}
		//i�Ԗڂɓ��I�����̂̃f�[�^(num)��ۑ�����
		for(j=0;j<geneSize;j++){
			for(int k=0;k<NOTE_CONTENTS_LENGTH;k++){
				newGenotype[i][j][k]=genotype[num][j][k];
			}
		}
		newFitness[i]=fitness[num];
	}
	copyNewToOld();	//�̌Q��V�����I�΂ꂽ�̌Q�ɂ���
}
//�ŗǌ̂�T��
void GA::findAndSetBestIndividual(){
	int i;
	double bestFitness;
	eliteNumber=0;
	bestFitness=fitness[0];
	
	//�ŗǌ̂�T��
	for(i=0;i<popSize;i++){
		if(bestFitness<fitness[i]){
			eliteNumber=i;
			bestFitness=fitness[i];
		}
	}
	updateElite=!(bestFitness==eliteFitness);
	if(updateElite){
		//�ŗǌ̂�ۑ�����
		for(i=0;i<geneSize;i++){
			for(int j=0;j<NOTE_CONTENTS_LENGTH;j++){
				eliteGenotype[i][j]=genotype[eliteNumber][i][j];
			}
		}
		if(eliteFitness<fitness[eliteNumber]){
			eliteGeneration=generation;
		}
		eliteFitness=fitness[eliteNumber];
		for(int i=0;i<SUPERORDINATE_FITNESS_LENGTH;i++){
			eliteDetailFitness[i]=detailFitness[eliteNumber][i];
		}
		for(int i=0;i<FITNESS_LENGTH;i++){
			eliteDetailMusicTheory[i]=detailMusicTheory[eliteNumber][i];
		}
		for(int i=0;i<GLOBAL_SIMILARITY_LENGTH;i++){
			eliteDetailGlobalSimilarity[i]=detailGlobalSimilarity[eliteNumber][i];
		}
	}
}
//�G���[�g�ۑ��헪���s��
void GA::elitistStrategy(void){
	int worstNumber,i;
	double worstFitness;

	//�ň��̂�T��
	worstNumber=0;
	worstFitness=fitness[0];
	for(i=0;i<popSize;i++){
		if(worstFitness>fitness[i]){
			worstNumber=i;
			worstFitness=fitness[i];
		}
	}
	//�O��̃G���[�g�̂ƍň��̂����ւ���
	for(i=0;i<geneSize;i++){
		for(int j=0;j<NOTE_CONTENTS_LENGTH;j++){
			genotype[worstNumber][i][j]=eliteGenotype[i][j];
		}
	}
	fitness[worstNumber]=eliteFitness;
	for(int i=0;i<SUPERORDINATE_FITNESS_LENGTH;i++){
		detailFitness[worstNumber][i]=eliteDetailFitness[i];
	}
}
//�œK�̂�\������
void GA::printBestIndividual(int n){
	cout << "No." << n << ": ";
	/*for(int i=0;i<geneSize;i++){
		for(int j=0;j<NOTE_CONTENTS_LENGTH;j++){
			cout << eliteGenotype[i][j];
		}
		cout << endl;
		if((i+1)%10==0){
			cout << " ";
		}
	}*/
	cout << eliteFitness << endl;
	cout << "  Music Theory: " << eliteDetailFitness[MUSIC_THEORY] << endl;
	for(int i=0;i<FITNESS_LENGTH;i++){
		cout << "\t" << FITNESS_NAME[i] << ": " << eliteDetailMusicTheory[i] << endl;
	}
	cout << "  Global: " << eliteDetailFitness[GLOBAL] << endl;
	cout << "  Local: " << eliteDetailFitness[LOCAL] << endl;
}
void GA::printBaseIndividual(){
	for(int i=0;i<1;i++){
		cout << "ID: " << i << endl;
		for(int j=0;j<geneSize;j++){
			for(int k=0;k<NOTE_CONTENTS_LENGTH;k++){
				cout << baseGenotype[i][j][k] << " ";
			}
			cout << ",";
			if(j%5==0){
				cout << endl;
			}
		}
	}
}
void GA::printParameters(void){}
//GA�Ŏg���z����m�ۂ���
void GA::allocateGA(void){
	long i;

	genotype = new int**[popSize];
	newGenotype = new int**[popSize];
	detailFitness = new double*[popSize];
	newDetailFitness = new double*[popSize];
	detailMusicTheory = new double*[popSize];
	detailGlobalSimilarity = new double*[popSize];
	for(i=0;i<popSize;i++){
		genotype[i]=new int*[geneSize];
		newGenotype[i] = new int*[geneSize];
		for(long j=0;j<geneSize;j++){
			genotype[i][j] = new int[NOTE_CONTENTS_LENGTH];
			newGenotype[i][j] = new int[NOTE_CONTENTS_LENGTH];
		}
		detailFitness[i] = new double[SUPERORDINATE_FITNESS_LENGTH];
		newDetailFitness[i] = new double[SUPERORDINATE_FITNESS_LENGTH];
		detailMusicTheory[i] = new double[FITNESS_LENGTH];
		detailGlobalSimilarity[i] = new double[GLOBAL_SIMILARITY_LENGTH];
	}
	fitness = new double[popSize];
	newFitness = new double[popSize];
	eliteGenotype = new int*[geneSize];
	for(i=0;i<geneSize;i++){
		eliteGenotype[i] = new int[NOTE_CONTENTS_LENGTH];
	}
	number = new int[popSize];
	rouletteTable = new double[popSize];
}
//GA�Ŏg���Ĕz����J������
void GA::deleteGA(void){
	for(long i=0;i<popSize;i++){
		for(long j=0;j<geneSize;j++){
			delete [] genotype[i][j];
			delete [] newGenotype[i][j];
		}
		delete [] genotype[i];
		delete [] newGenotype[i];
		delete [] detailFitness[i];
		delete [] newDetailFitness[i];
		delete [] detailMusicTheory[i];
		delete [] detailGlobalSimilarity[i];
	}
	for(int i=0;i<baseLength;i++){
		for(int j=0;j<baseMusicLength[i];j++){
			delete [] baseGenotype[i][j];
		}
		delete [] baseGenotype[i];
	}
	for(int i=0;i<geneSize;i++){
		delete [] eliteGenotype[i];
	}
	delete [] genotype;
	delete [] newGenotype;
	delete [] baseGenotype;
	delete [] fitness;
	delete [] newFitness;
	delete [] detailFitness;
	delete [] newDetailFitness;
	delete [] detailMusicTheory;
	delete [] detailGlobalSimilarity;
	delete [] eliteGenotype;
	delete [] number;
	delete [] rouletteTable;
	delete [] baseMusicLength;
}
void GA::setGA(){}
void GA::startGA(void){}
void GA::initializeParameters(void){}
void GA::initializeGenes(void){}
int GA::getPopSize(){
	return popSize;
}
int GA::getMusicLength(){
	return geneSize;
}
int GA::getMaxGeneration(){
	return MAX_GENERATION;
}
double GA::getCrossoverRate(){
	return crossoverRate;
}
double GA::getMutationRate(){
	return mutationRate;
}
int** GA::getBestIndividual(){
	return eliteGenotype;
}
int GA::getEliteGeneration(){
	return eliteGeneration;
}
double* GA::getEliteDetailFitness(){
	return eliteDetailFitness;
}
double* GA::getEliteDetailMusicTheory(){
	return eliteDetailMusicTheory;
}
double* GA::getEliteDetailGlobalSimilarity(){
	return eliteDetailGlobalSimilarity;
}
double GA::getPitchRange(){
	return theorist.getPitchRange();
}
double* GA::getImpulseRate(){
	return theorist.getImpulseRate();
}
double GA::getAverageImpulseRate(){
	return theorist.getAverageImpulseRate();
}
double GA::getOverAverageImpulseRate(){
	return theorist.getOverAverageImpulseRate();
}
void GA::calcFitness(){
	theorist.setGene(genotype);
	executeLS=(generation==1 || generation%EXECUTE_LS_SPAN==0 || generation>MAX_GENERATION-EXECUTE_FINAL_LS || checkLocalSimilarity());
	theorist.calcFitness(executeLS);
}
void GA::printSheet(){
	double fitnesses[4];
	fitnesses[0]=eliteFitness;
	for(int i=0;i<SUPERORDINATE_FITNESS_LENGTH;i++){
		fitnesses[i+1]=eliteDetailFitness[i];
	}
	fileManager.saveCSV(fitnesses,4);
}
void GA::printAverageDetailFitness(){
	for(int i=0;i<SUPERORDINATE_FITNESS_LENGTH;i++){
		double average=0;
		for(int j=0;j<popSize;j++){
			average+=detailFitness[j][i];
		}
		cout << "ave.: " << average/popSize << endl;
	}
}
bool GA::checkLocalSimilarity(){
	double average=0;
	for(int i=0;i<popSize;i++){
		average+=detailFitness[i][LOCAL];
	}
	if(average/popSize<thresholdExecuteLocalSim){
		return true;
	}
	return false;
}