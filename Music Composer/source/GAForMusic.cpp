#include "GAForMusic.h"
#include "Measure.h"

GAForMusic::GAForMusic(void)
{
	localSimilarity=0;
}
GAForMusic::~GAForMusic(void)
{
}
void GAForMusic::setGA(){
	initializeParameters();	//パラメータの初期化
	setMedianTone();
	theorist.setMusicLength(geneSize);
	theorist.setPopSize(popSize);
	theorist.setMaxSimilarity();
	theorist.initParameters();
	theorist.printFitnessWeight();
	printParameters();	//パラメータの表示
	initializeGenes();	//染色体の初期化
	theorist.setFitness(fitness,detailFitness,detailMusicTheory,detailGlobalSimilarity);
	
	Measure m;
	m.showMemory();

	generation=1;
	calcFitness();
	findAndSetBestIndividual();
	printBestIndividual(1);
	printAverageDetailFitness();
	printSheet();
}
void GAForMusic::startGA(void){
	cout << "---Start GA---" << endl;
	do{
		bool prevExecuteLS=executeLS;
		generation++;
		selectionUsingRouletteRule();
		executeCrossover();
		executeProcessing();
		calcFitness();
		if(eliteFlag /*&& !(!prevExecuteLS && executeLS)*/){
			elitistStrategy();
		}
		findAndSetBestIndividual();
		if(generation%PRINT_SPAN==0){
			printBestIndividual(generation);
			printAverageDetailFitness();
		}
		//if(generation%5==0){
			printSheet();
		//}
	}while(generation<MAX_GENERATION || (generation<(double)MAX_GENERATION*1.0 && !meetTheory()));
	cout << "---Optimum Solution---" << endl;
	theorist.calcFitnessForIndividual(eliteGenotype,eliteNumber);
	findAndSetBestIndividual();
	printBestIndividual(eliteGeneration);
	
	//(*this).deleteGA();
}
//GAのパラメータの初期化
void GAForMusic::initializeParameters(void){
	geneSize=GENE_SIZE;
	popSize = POP_SIZE;
	crossType = MULTI_POINT;
	crossoverRate = CROSSOVER_RATE;
	mutationRate = MUTATION_RATE;
	eliteFlag = TRUE;
	selfCopyRate = SELF_COPY_RATE;
	measureLength = geneSize/BASE_NUM;
}
//染色体の初期化
void GAForMusic::initializeGenes(void){
	(*this).allocateGA();

	//double keyFreq[BASE_NUM+1][KEY_NUM];
	//double durationFreq[BASE_NUM+1][BASE_NUM];

	double ***keyFreq = new double**[baseLength];
	double ***durationFreq = new double**[baseLength];

	for(int i=0;i<baseLength;i++){
		keyFreq[i] = new double*[BASE_NUM+1];
		durationFreq[i] = new double*[BASE_NUM+1];
		for(int j=0;j<BASE_NUM+1;j++){
			keyFreq[i][j] = new double[KEY_NUM];
			durationFreq[i][j] = new double[BASE_NUM];
			for(int k=0;k<KEY_NUM;k++){
				keyFreq[i][j][k]=0;
			}
			for(int k=0;k<BASE_NUM;k++){
				durationFreq[i][j][k]=0;
			}
		}
	}

	int **base;
	int baseLen;
	int count;
	int maxKey=0,minKey=KEY_NUM;
	int interval;
	for(int i=0;i<baseLength;i++){
		base=baseGenotype[i];
		baseLen=baseMusicLength[i];
		count=0;
		for(int j=0;j<baseLen;j++){
			if(base[j][NOTE_RISING]==0){
				maxKey=max(maxKey,base[j][NOTE_KEY]);
				minKey=min(minKey,base[j][NOTE_KEY]);
				keyFreq[i][j%BASE_NUM][base[j][NOTE_KEY]]++;
				keyFreq[i][BASE_NUM][base[j][NOTE_KEY]]++;
				if(count<BASE_NUM){
					durationFreq[i][j%BASE_NUM][count]++;
					durationFreq[i][BASE_NUM][count]++;
				}
				count=0;
			}else{
				count++;
			}
		}
	}
	interval=maxKey-minKey;
	for(int i=0;i<baseLength;i++){
		for(int j=0;j<BASE_NUM+1;j++){
			double sum=0;
			for(int k=0;k<KEY_NUM;k++){
				sum+=keyFreq[i][j][k];
			}
			for(int k=0;k<KEY_NUM;k++){
				if(sum==0){
					keyFreq[i][j][k]=0;
				}else{
					keyFreq[i][j][k]/=sum;
				}
				//cout << "keyFreq " << keyName[j%SCALE_LENGTH] << " : " << keyFreq[i][j] << endl;
			}
			sum=0;
			for(int k=0;k<KEY_NUM;k++){
				sum+=keyFreq[i][j][k];
				keyFreq[i][j][k]=sum;
			}
			sum=0;
			for(int k=0;k<BASE_NUM;k++){
				sum+=durationFreq[i][j][k];
			}
			for(int k=0;k<BASE_NUM;k++){
				if(sum==0){
					durationFreq[i][j][k]=0;
				}else{
					durationFreq[i][j][k]/=sum;
				}
				//cout << "durationFreq " << durationFreq[i][j] << endl;
			}
			sum=0;
			for(int k=0;k<BASE_NUM;k++){
				sum+=durationFreq[i][j][k];
				durationFreq[i][j][k]=sum;
			}
		}
	}															
	for(int i=0;i<popSize;i++){
		int prevKey=SCALE_LENGTH*5;
		for(int j=0;j<geneSize;j++){
			int id=random.nextInt(baseLength+1);
			int key=0;
			int duration=0;
			if(id<baseLength){
				double hit =random.nextDouble(1.0);
				if(keyFreq[id][j%BASE_NUM][KEY_NUM-1]!=0 && random.nextDouble(1)<0.8){
					while(hit>keyFreq[id][j%BASE_NUM][key]){
						key++;
					}
				}else{
					while(hit>keyFreq[id][BASE_NUM][key]){
						key++;
					}
				}
				//hit=random.nextInt(3);
				//key=(hit+3)*SCALE_LENGTH;
				//key+=SCALE_LENGTH;
				prevKey=key;
				hit =random.nextDouble(1.0);
				if(durationFreq[id][j%BASE_NUM][BASE_NUM-1]!=0 && random.nextDouble(1)<0.8){
					while(hit>durationFreq[id][j%BASE_NUM][duration]){
						duration++;
					}
				}else{
					while(hit>durationFreq[id][BASE_NUM][duration]){
						duration++;
					}
				}
			}else{
				do{
					key=minKey+random.nextInt(interval);
				}while(pattern[key%SCALE_LENGTH]);
				//int duration = pow(2.0,random.nextInt(2)+1);
				duration = pow(2.0,random.nextInt(4))-1;
				duration=random.nextInt(BASE_NUM);
			}
			for(int k=0;j+k<geneSize && k<duration+1;k++){
				genotype[i][j+k][NOTE_KEY]=key;
				genotype[i][j+k][NOTE_RISING]=k;
			}
			j+=duration;
		}
	}


	/*for(int i=0;i<BASE_NUM+1;i++){
		for(int j=0;j<KEY_NUM;j++){
			keyFreq[i][j]=0;
		}
		for(int j=0;j<BASE_NUM;j++){
			durationFreq[i][j]=0;
		}
	}
	int **base;
	int baseLen;
	int count;
	for(int i=0;i<baseLength;i++){
		base=baseGenotype[i];
		baseLen=baseMusicLength[i];
		count=0;
		for(int j=0;j<baseLen;j++){
			if(base[j][NOTE_RISING]==0){
				keyFreq[j%BASE_NUM][base[j][NOTE_KEY]]++;
				keyFreq[BASE_NUM][base[j][NOTE_KEY]]++;
				if(count<BASE_NUM && count!=0){
					durationFreq[j%BASE_NUM][count]++;
					durationFreq[BASE_NUM][count]++;
				}
				count=0;
			}else{
				count++;
			}
		}
	}
	for(int i=0;i<BASE_NUM+1;i++){
		double sum=0;
		for(int j=0;j<KEY_NUM;j++){
			sum+=keyFreq[i][j];
		}
		for(int j=0;j<KEY_NUM;j++){
			if(sum==0){
				keyFreq[i][j]=0;
			}else{
				keyFreq[i][j]/=sum;
			}
			//cout << "keyFreq " << keyName[j%SCALE_LENGTH] << " : " << keyFreq[i][j] << endl;
		}
		sum=0;
		for(int k=0;k<KEY_NUM;k++){
			sum+=keyFreq[i][k];
			keyFreq[i][k]=sum;
		}
		sum=0;
		for(int j=0;j<BASE_NUM;j++){
			sum+=durationFreq[i][j];
		}
		for(int j=0;j<BASE_NUM;j++){
			if(sum==0){
				durationFreq[i][j]=0;
			}else{
				durationFreq[i][j]/=sum;
			}
			//cout << "durationFreq " << durationFreq[i][j] << endl;
		}
		sum=0;
		for(int k=0;k<BASE_NUM;k++){
			sum+=durationFreq[i][k];
			durationFreq[i][k]=sum;
		}
	}
	for(int i=0;i<popSize/2;i++){
		int prevKey=SCALE_LENGTH*5;
		for(int j=0;j<geneSize;j++){
			double hit =random.nextDouble(1.0);
			int key=0;
			if(keyFreq[j%BASE_NUM][KEY_NUM-1]!=0 && random.nextDouble(1)<0.8){
				while(hit>keyFreq[j%BASE_NUM][key]){
					key++;
				}
			}else{
				while(hit>keyFreq[BASE_NUM][key]){
					key++;
				}
			}
			//hit=random.nextInt(3);
			//key=(hit+3)*SCALE_LENGTH;
			//key+=SCALE_LENGTH;
			prevKey=key;
			hit =random.nextDouble(1.0);
			int duration=0;
			if(durationFreq[j%BASE_NUM][BASE_NUM-1]!=0 && random.nextDouble(1)<0.8){
				while(hit>durationFreq[j%BASE_NUM][duration]){
					duration++;
				}
			}else{
				while(hit>durationFreq[BASE_NUM][duration]){
					duration++;
				}
			}
			for(int k=0;j+k<geneSize && k<duration+1;k++){
				genotype[i][j+k][NOTE_KEY]=key;
				genotype[i][j+k][NOTE_RISING]=k;
			}
			j+=duration;
		}
	}*/

	/*for(int i=0;i<popSize/2;i++){
		for(int j=0;j<geneSize;j++){
			int individual = random.nextInt(baseLength);
			int note;
			int serial = random.nextInt(MAX_SERIAL)+1;
			//if(i<popSize/2){
				//do{
					//note = random.nextInt(baseMusicLength[individual]);
				//}while(note%BASE_NUM!=j%BASE_NUM);
			//}else{
			//	note = random.nextInt(baseMusicLength[individual]);
			//}
			do{
				note = random.nextInt(baseMusicLength[individual]);
			}while(baseGenotype[individual][note][NOTE_RISING]);

			int startNote=note;
			for(int n=0;n<serial;n++){
				do{
					for(int k=0;k<NOTE_CONTENTS_LENGTH;k++){
						genotype[i][j][k]=baseGenotype[individual][note][k];
					}
					if(note==startNote){
						genotype[i][j][NOTE_RISING]=0;
					}
					note++;
					j++;
				}while(j<geneSize && note<baseMusicLength[individual] && baseGenotype[individual][note][NOTE_RISING]!=0);
				if(j>=geneSize || note>=baseMusicLength[individual]){
					break;
				}
			}
			j--;
		}
	}*/
	
	/*
	//tmp
		minKey=57;
		interval=81-minKey;
		for(int i=0;i<popSize;i++){
			for(int j=0;j<geneSize;j++){
				int key;
				int duration;
				do{
					key=minKey+random.nextInt(interval);
				}while(pattern[key%SCALE_LENGTH]);
				//int duration = pow(2.0,random.nextInt(2)+1);
				duration=random.nextInt(BASE_NUM);
				for(int n=0;j<geneSize && n<duration;n++,j++){
					genotype[i][j][NOTE_KEY]=key;
					genotype[i][j][NOTE_RISING]=n;
				}
				j--;
			}
		}
	//tmp
	*/

	for(int i=0;i<baseLength;i++){
		for(int j=0;j<BASE_NUM+1;j++){
			delete [] keyFreq[i][j];
			delete [] durationFreq[i][j];
		}
		delete [] keyFreq[i];
		delete [] durationFreq[i];
	}
	delete [] keyFreq;
	delete [] durationFreq;

	for(int i=0;i<popSize;i++){
		individual=genotype[i];
		modifyMelodyLine();
		modifyDuration();
	}
}
void GAForMusic::setBase(int ***base,int *musicLength,int length,int scale){
	cout << "---Set Base---" << endl;
	baseLength=length;
	baseMusicLength=musicLength;
	baseGenotype = base;
	(*this).scale=scale;
	int ptn[SCALE_LENGTH] = {0,1,0,1,0,0,1,0,1,0,1,0};
	for(int i=0;i<SCALE_LENGTH;i++){
		pattern[i]=ptn[i];
	}
	theorist.setBase(baseGenotype,baseLength,baseMusicLength,scale);
}
void GAForMusic::executeProcessing(){
	for(int i=0;i<popSize;i++){
		individual=genotype[i];
		executeMutation();
		modifyMelodyLine();
		modifyDuration();
	}
}
void GAForMusic::executeCrossover(){
	int i,num1,num2;
	int numOfPair;	//ペアの総数

	//初期化(すべての親の番号を配列に入れる)
	for(i=0;i<popSize;i++){
		number[i]=i;
	}
	//並べ替え
	for(i=0;i<popSize*10;i++){
		num1=random.nextInt(popSize);
		num2=random.nextInt(popSize);
		swapInt(&number[num1],&number[num2]);
	}

	//ペアの数は個体数の半分
	numOfPair=popSize/2;

	for(i=0;i<numOfPair;i++){
		num1=number[2*i];
		num2=number[2*i+1];
		if(random.nextDouble(1.0)<=crossoverRate){
			switch(crossType){
			case ONE_POINT:
				onePointCrossover(num1,num2);
				break;
			case TWO_POINT:
				twoPointCrossover(num1,num2);
				break;
			case MULTI_POINT:
				multiPointCrossover(num1,num2);
				break;
			case UNIFORM:
				uniformCrossover(num1,num2);
				break;
			}
			/*if(!executeLS){
				double localSimilarity=(detailFitness[num1][LOCAL]+detailFitness[num2][LOCAL])/2;
				detailFitness[num1][LOCAL]=localSimilarity;
				detailFitness[num2][LOCAL]=localSimilarity;
			}*/
		}
	}
}
void GAForMusic::executeMutation(){
	if(random.nextDouble(1.0)<=mutationRate){
		if(random.nextDouble(1.0)<=selfCopyRate){
			selfCopy();
		}else{
			for(int i=0;i<geneSize;i++){
				if(random.nextDouble(1.0)<=EXECUTE_MUTATION){
					individual[i][NOTE_KEY]+=random.nextInt(SCALE_LENGTH)-random.nextInt(SCALE_LENGTH);
					individual[i][NOTE_KEY]=min(medianTone+SCALE_LENGTH,max(individual[i][NOTE_KEY],medianTone-SCALE_LENGTH));
					if(random.nextDouble(1.0)<=0.5){
						individual[i][NOTE_RISING]=random.nextInt(2);
					}
				}
			}
		}	
	}
}
void GAForMusic::multiPointCrossover(int n1,int n2){
	int crossPoint;
	int start,end;
	double probability=min(fitness[n1],fitness[n2]);
	for(int i=0;i<measureLength;i++){
		//if(random.nextDouble(2-eliteFitness)>probability){
		if(random.nextDouble(probability)<INVERSE_MEASURE_UNIT){
			crossPoint = random.nextInt(BASE_NUM);
			if(random.nextInt(1)){
				start=0;
				end=crossPoint+1;
			}else{
				start=crossPoint+1;
				end=BASE_NUM;
			}
			for(int j=start;j<end;j++){
				for(int k=0;k<NOTE_CONTENTS_LENGTH;k++){
					swapInt(&genotype[n1][i*BASE_NUM+j][k], &genotype[n2][i*BASE_NUM+j][k]);
				}
			}
		}
	}	
}
void GAForMusic::selfCopy(){
	int baseStart;
	int copiedStart;
	baseStart=random.nextInt((geneSize-BASE_NUM)/BASE_NUM);
	copiedStart=random.nextInt((geneSize-BASE_NUM)/BASE_NUM);
	int count=0;
	for(int i=0;i<BASE_NUM;i++){
		if(!individual[baseStart*BASE_NUM+i][NOTE_RISING]){
			count++;
		}
	}
	int start=random.nextInt(BASE_NUM);
	int end=start+random.nextInt(BASE_NUM-start);
	int choice = random.nextInt(2);
	for(int i=start;i<end;i++){
		if(choice){
			for(int j=0;j<NOTE_CONTENTS_LENGTH;j++){
				individual[copiedStart*BASE_NUM+i][j]=individual[baseStart*BASE_NUM+i][j];
			}
		}else{
			individual[copiedStart*BASE_NUM+i][NOTE_RISING]=individual[baseStart*BASE_NUM+i][NOTE_RISING];
		}
	}
}
void GAForMusic::modifyMelodyLine(){
	for(int i=0;i<geneSize-1;i++){
		int prev=individual[i][NOTE_KEY];
		do{
			i++;
		}while(i<geneSize-1 && individual[i][NOTE_RISING]);
		int dif=prev-individual[i][NOTE_KEY];
		if(dif>SCALE_LENGTH){
			individual[i][NOTE_KEY]=prev-dif%SCALE_LENGTH;
		}else if(dif<-SCALE_LENGTH){
			individual[i][NOTE_KEY]=prev+(SCALE_LENGTH-dif%SCALE_LENGTH);
		}
		i--;
	}
}
void GAForMusic::modifyDuration(){
	for(int i=0;i<geneSize;i++){
		int count=0;
		do{
			count++;
			if(count>BASE_NUM){
				for(int j=0;j<BASE_NUM;j+=2){
					individual[i-j][NOTE_RISING]=0;
				}
				break;
			}
			i++;
		}while(i<geneSize && individual[i][NOTE_RISING]);
		i--;
	}
}
void GAForMusic::setMedianTone(){
	int keyFreq[KEY_NUM];
	for(int i=0;i<KEY_NUM;i++){
		keyFreq[i]=0;
	}
	int count=0;
	for(int i=0;i<baseLength;i++){
		for(int j=0;j<baseMusicLength[i];j++){
			if(keyFreq[baseGenotype[i][j][NOTE_KEY]]==0){
				count++;
			}
			keyFreq[baseGenotype[i][j][NOTE_KEY]]++;
		}
	}
	int check=0;
	for(int i=0;i<KEY_NUM;i++){
		if(keyFreq[i]){
			check++;
			if(check>count/2){
				medianTone=i;
				break;
			}
		}
	}
	cout << "median" << medianTone << endl;
}
//GAのパラメータの表示
void GAForMusic::printParameters(void){
	cout << "---GA Parameters---" << endl;
	cout << "Population Size: " << popSize << endl;
	cout << "Gene Size: " << geneSize << endl;
	cout << "Max Generation: " << MAX_GENERATION << endl;
	cout << "Crossover Type: ";
	switch(crossType){
	case ONE_POINT:
		cout << "one-point" << endl;
		break;
	case TWO_POINT:
		cout << "two-point" << endl;
		break;
	case MULTI_POINT:
		cout << "multi-point" << endl;
		break;
	case UNIFORM:
		cout << "uniform" << endl;
		break;
	}
	cout << "Crossover Rate: " << crossoverRate << endl;
	cout << "Mutation Rate: " << mutationRate << endl;
	cout << "SelfCopy Rate: " << selfCopyRate << endl;
	cout << "Elite Strategy: ";
	if(eliteFlag){
		cout << "true" << endl;
	}else{
		cout << "false" << endl;;
	}
}
double GAForMusic::getSelfCopyRate(){
	return selfCopyRate;
}
double* GAForMusic::getFitnessWeight(){
	return theorist.getFitnessWeight();
}
double GAForMusic::getLocalSimilarity(){
	return localSimilarity;
}
bool GAForMusic::meetTheory(){
	if(generation!=MAX_GENERATION && !updateElite){
		return false;
	}
	theorist.setIndividual(eliteGenotype);
	localSimilarity=theorist.localSimilarityForIndividual();
	cout << "Elite Generation: " << eliteGeneration << " Local Similarity: " << localSimilarity << endl;
	return (localSimilarity>0.95);
}