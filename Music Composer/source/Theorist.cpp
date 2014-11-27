#include "Theorist.h"
#include <cmath>

Theorist::Theorist(void)
{
	for(int i=0;i<FITNESS_LENGTH;i++){
		fitnessWeight[i]=0;
	}
	for(int i=0;i<BASE_NUM;i++){
		impulseRate[i]=0;
		
	}
	int ptn[SCALE_LENGTH] = {0,1,0,1,0,0,1,0,1,0,1,0};
	for(int i=0;i<5;i++){
		pattern[i]=i%2;
	}
	for(int i=0;i<7;i++){
		pattern[i+5]=i%2;
	}
	rsCheck=false;
	superordinateFitnessWeight[MUSIC_THEORY]=1;
	superordinateFitnessWeight[GLOBAL]=0;
	superordinateFitnessWeight[LOCAL]=0;
	globalSimilarityFunctions[CHORD_SIMILARITY]=&Theorist::chordSimilarity;
	//globalSimilarityFunctions[CHORD_SIMILARITY]=&Theorist::returnZero;
	//globalSimilarityFunctions[IMPULSE_SIMILARITY]=&Theorist::impulseSimilarity;
	
	fitnessFunctions[SMOOTH]=&Theorist::smooth;
	fitnessFunctions[HARMONY]=&Theorist::harmony;
	fitnessFunctions[SD]=&Theorist::standardDeviationOfMelody;
	fitnessFunctions[RHYTHMIC_CONTINUITY]=&Theorist::rhythmicContinuity;
	fitnessFunctions[CHORD_ADAPTATION]=&Theorist::chordAdaptation;
	fitnessFunctions[IMPLUSE]=&Theorist::impulseSimilarity;

	fitnessForBaseFunctions[SMOOTH]=&Theorist::smoothForBase;
	fitnessForBaseFunctions[HARMONY]=&Theorist::harmonyForBase;
	fitnessForBaseFunctions[SD]=&Theorist::returnZero;
	fitnessForBaseFunctions[SD]=&Theorist::returnPlus;
	//fitnessForBaseFunctions[SD]=&Theorist::standardDeviationOfMelodyForBase;
	fitnessForBaseFunctions[RHYTHMIC_CONTINUITY]=&Theorist::rhythmicContinuityForBase;
	//fitnessForBaseFunctions[RHYTHMIC_CONTINUITY]=&Theorist::returnMinus;
	fitnessForBaseFunctions[CHORD_ADAPTATION]=&Theorist::chordAdaptationForBase;
	fitnessForBaseFunctions[IMPLUSE]=&Theorist::returnPlus;
	fitnessForBaseFunctions[IMPLUSE]=&Theorist::impulseSimilarityForBase;
}
Theorist::~Theorist(void)
{
	delete [] chords;
	delete [] similarity;
	delete [] series;
	for(int i=0;i<baseLength;i++){
		delete [] baseDetailFitness[i];
	}
	delete [] baseDetailFitness;
}
void Theorist::setBase(int ***base, int baseLength,int *baseMusicLength,int scale){
	this->baseLength = baseLength;
	this->baseMusicLength = baseMusicLength;
	baseGenotype = base;
	baseDetailFitness = new double*[baseLength];
	for(int i=0;i<baseLength;i++){
		baseDetailFitness[i] = new double[FITNESS_LENGTH];
	}
	this->scale=scale;
	for(int i=0;i<baseLength;i++){
		cout << "base music length: " << baseMusicLength[i] << endl;
	}
	setStandardDeviationOfMelodyForBase();
	setImpulseRate();
	setBaseChord();
	//printChord();
}
void Theorist::setGene(int ***gene){
	genotype=gene;
}
void Theorist::setIndividual(int **individual){
	this->individual=individual;
}
void Theorist::setMusicLength(int musicLength){
	this->musicLength = musicLength;
	chordLength=musicLength/BASE_NUM;
}
void Theorist::setPopSize(int pop){
	popSize=pop;
	chords = new MusicChord*[popSize];
	similarity = new double[popSize];
	series = new int[popSize];
}
void Theorist::setFitness(double *fitness,double **detailFitness,double **detailMusicTheory,double **detailGlobalSimilarity){
	this->fitness=fitness;
	this->detailFitness=detailFitness;
	this->detailMusicTheory=detailMusicTheory;
	this->detailGlobalSimilarity=detailGlobalSimilarity;
}
void Theorist::setMaxSimilarity(){
	maxGlobalSimilarity=0;
	maxLocalSimilarity=0;
	for(int i=0;i<baseLength;i++){
		maxGlobalSimilarity+=baseMusicLength[i];
		maxLocalSimilarity+=baseMusicLength[i];
	}
	maxGlobalSimilarity=maxGlobalSimilarity*musicLength/BASE_NUM/BASE_NUM/10;
	maxLocalSimilarity=maxLocalSimilarity*musicLength/20;
	cout << maxGlobalSimilarity << endl;
	cout << maxLocalSimilarity << endl;
}
void Theorist::setMaxFitnesses(){
	maxFitnesses[SMOOTH]=(musicLength-1)*SCALE_LENGTH;
	maxFitnesses[HARMONY]=musicLength*THRESHOLD_RATE[HARMONY];
	maxFitnesses[SD]=1;
	maxFitnesses[RHYTHMIC_CONTINUITY]=(musicLength-1)*4;
	maxFitnesses[CHORD_ADAPTATION]=1;
	//maxFitnesses[HARMONY_IN_CHORD]=1;
}
void Theorist::initParameters(){
	setMedianTone();
	setMaxFitnesses();
	calcBaseFitness();
	setFitnessWeight();
	printBaseFitness();
}
void Theorist::setMedianTone(){
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
void Theorist::setChord(){
	chordManager.setMusicLength(musicLength);
	for(int i=0;i<popSize;i++){
		chordManager.setNotes(genotype[i]);
		chords[i] = chordManager.judgeChord();
	}
}
void Theorist::calcFitness(bool executeLS){
	setChord();
	musicTheory();
	globalSimilarity();
	if(executeLS){
		//localSimilarity();
	}
	for(int i=0;i<popSize;i++){
		fitness[i]=0;
		for(int j=0;j<SUPERORDINATE_FITNESS_LENGTH;j++){
			if(j!=LOCAL || executeLS){
				detailFitness[i][j]=getSuperordinateFitnessWeight(j)*detailFitness[i][j];
			}
			fitness[i]+=detailFitness[i][j];
		}
		delete [] chords[i];
	}
}
void Theorist::musicTheory(){
	double *fitness;
	double *musicTheory;
	double value;
	for(int i=0;i<popSize;i++){
		individual=genotype[i];
		chord=chords[i];
		fitness=detailFitness[i];
		musicTheory=detailMusicTheory[i];
		fitness[MUSIC_THEORY]=0;
		for(int j=0;j<FITNESS_LENGTH;j++){
			value = fitnessWeight[j]*(this->*fitnessFunctions[j])();
			musicTheory[j]=value;
			fitness[MUSIC_THEORY]+=value;
		}
		fitness[MUSIC_THEORY]+=1;
	}
}
void Theorist::globalSimilarity(){
	double value;
	double *fitness;
	for(int i=0;i<popSize;i++){
		individual=genotype[i];
		chord=chords[i];
		fitness=detailFitness[i];
		fitness[GLOBAL]=0;
		for(int j=0;j<GLOBAL_SIMILARITY_LENGTH;j++){
			value=(this->*globalSimilarityFunctions[j])();
			detailGlobalSimilarity[i][j]=value;
			fitness[GLOBAL]+=value;
		}
	}
}
double Theorist::chordSimilarity(){
	MusicChord *base;
	double similarity=0;
	int count=0;
	/*for(int n=0;n<baseLength;n++){
		int baseChordLength=baseMusicLength[n]/BASE_NUM;
		int start=chordLength-2;
		base = baseChord[n];
		for(int i=-start;i<baseChordLength;i++){
			int chordSeries=0;
			int twice=1;
			if(i%(BASE_NUM/2)==0){
				twice=2;
			}
			for(int j=0;start+j<chordLength && start+i+j<baseChordLength;j++){
				if(chord[start+j].getRoot()==base[start+i+j].getRoot() && chord[start+j].getQuality()==base[start+i+j].getQuality()){
					chordSeries++;
				}else{
					if(chordSeries>1){
						count++;
						similarity+=twice*pow(2.0,chordSeries-1);
					}else{
						chordSeries=max(chordSeries-1,0);
					}
				}
			}
			start=max(0,start-1);
		}
	}*/
	for(int n=0;n<baseLength;n++){
		int baseChordLength=baseMusicLength[n]/BASE_NUM;
		int start=chordLength-4;
		base = baseChord[n];
		for(int i=-start;i<baseChordLength;i+=4){
			int chordSeries=0;
			for(int j=0;start+j<chordLength && start+i+j<baseChordLength;j++){
				if(chord[start+j].getRoot()==base[start+i+j].getRoot() && chord[start+j].getQuality()==base[start+i+j].getQuality()){
					chordSeries++;
				}else{
					if(chordSeries>1){
						count++;
						similarity+=pow(2.0,chordSeries-1);
					}else{
						chordSeries=max(chordSeries-1,0);
					}
				}
			}
			start=max(0,start-1);
		}
	}
	return min(similarity,maxGlobalSimilarity)/maxGlobalSimilarity;
}
double Theorist::impulseSimilarity(){
	double fitness=0;
	double rate=int(musicLength/BASE_NUM)*overAverageImpulseRate;
	for(int i=0;i<musicLength;i++){
		if(!individual[i][NOTE_RISING]){
			if(averageImpulseRate<=impulseRate[i%BASE_NUM]){
				fitness+=impulseRate[i%BASE_NUM];
			}else{
				fitness-=(averageImpulseRate-impulseRate[i%BASE_NUM]);
			}
		}
	}
	fitness=min(1.0,fitness/rate);
	return -1+fitness;
}
double Theorist::impulseSimilarityForBase(){
	/*if(rsCheck){
		return baseDetailFitness[0][IMPLUSE];
	}*/
	double fitness=0;
	double rate=int(baseIndividualLength/BASE_NUM)*overAverageImpulseRate;
	for(int i=0;i<baseIndividualLength;i++){
		if(!baseIndividual[i][NOTE_RISING]){
			if(averageImpulseRate<=impulseRate[i%BASE_NUM]){
				fitness+=impulseRate[i%BASE_NUM];
			}else{
				fitness-=(averageImpulseRate-impulseRate[i%BASE_NUM]);
			}
		}
	}
	fitness=min(1.0,fitness/rate);
	rsCheck=true;
	return fitness;
}
void Theorist::localSimilarity(){
	for(int i=0;i<popSize;i++){
		similarity[i]=0;
		series[i]=0;
	}
	int **base;
	int *baseNote;
	int *individualNote;
	for(int n=0;n<baseLength;n++){
		base=baseGenotype[n];
		int start=musicLength-BASE_NUM;	
		for(int i=-start;i<baseMusicLength[n];i++){
			for(int j=0;start+j<musicLength && start+i+j<baseMusicLength[n];j++){
				baseNote=base[start+i+j];
				int twice=1;
				if(i%BASE_NUM==0){
					twice=2;
				}
				for(int k=0;k<popSize;k++){
					if(similarity[k]>=maxLocalSimilarity){
						continue;
					}
					if(i==-start){
						series[k]=0;
					}
					individualNote=genotype[k][start+j];
					if(individualNote[NOTE_RISING]!=0 && baseNote[NOTE_RISING]!=0){
						continue;
					}
					if(individualNote[NOTE_RISING]==0 && baseNote[NOTE_RISING]==0){
						if(individualNote[NOTE_KEY]==baseNote[NOTE_KEY]){
							series[k]++;
						}else{
							if(series[k]>4){
								similarity[k]+=twice*pow(2.0,series[k]-4);
							}
						}
					}else{
						series[k]=max(series[k]-1,0);
					}
				}
			}
			start=max(0,start-1);
		}
	}
	for(int i=0;i<popSize;i++){
		detailFitness[i][LOCAL]=1-min(similarity[i],maxLocalSimilarity)/maxLocalSimilarity;
	}
}
void Theorist::calcFitnessForIndividual(int **gene,int id){
	individual = gene;
	chordManager.setNotes(individual);
	chord = chordManager.judgeChord();
	musicTheoryForIndividual(id);
	globalSimilarityForIndividual(id);
	detailFitness[id][LOCAL]=localSimilarityForIndividual();
	fitness[id]=0;
	for(int i=0;i<SUPERORDINATE_FITNESS_LENGTH;i++){
		detailFitness[id][i]=getSuperordinateFitnessWeight(i)*detailFitness[id][i];
		fitness[id]+=detailFitness[id][i];
	}
}
void Theorist::musicTheoryForIndividual(int id){
	detailFitness[id][MUSIC_THEORY]=0;
	for(int i=0;i<FITNESS_LENGTH;i++){
		double value = fitnessWeight[i]*(this->*fitnessFunctions[i])();
		detailMusicTheory[id][i]=value;
		detailFitness[id][MUSIC_THEORY]+=value;
	}
	detailFitness[id][MUSIC_THEORY]+=1;
}
void Theorist::globalSimilarityForIndividual(int id){
	detailFitness[id][GLOBAL]=0;
	for(int i=0;i<GLOBAL_SIMILARITY_LENGTH;i++){
		double value=(this->*globalSimilarityFunctions[i])()*0.5;
		detailGlobalSimilarity[id][i]=value;
		detailFitness[id][GLOBAL]+=value;
	}
}
double Theorist::localSimilarityForIndividual(){
	double similarity=0;
	int series=0;
	int **base;
	int *baseNote;
	int *individualNote;
	for(int n=0;n<baseLength;n++){
		base=baseGenotype[n];
		int start=musicLength-BASE_NUM;
		int baseMusicLen=baseMusicLength[n];
		for(int i=-start;i<baseMusicLen;i++){
			for(int j=0;start+j<musicLength && start+i+j<baseMusicLen;j++){
				baseNote=base[start+i+j];
				individualNote=individual[start+j];
				if(individualNote[NOTE_RISING]!=0 && baseNote[NOTE_RISING]!=0){
					continue;
				}
				int twice=1;
				if(i%BASE_NUM==0){
					twice=2;
				}
				if(individualNote[NOTE_RISING]==0 && baseNote[NOTE_RISING]==0){
					if(individualNote[NOTE_KEY]==baseNote[NOTE_KEY]){
						series++;
					}else{
						if(series>4){
							similarity+=twice*pow(2.0,series-4);
						}
					}
				}else{
					series=max(series-1,0);
				}
				
			}
			start=max(0,start-1);
		}
	}
	return 1-min(similarity,maxLocalSimilarity)/maxLocalSimilarity;
}
double Theorist::getSuperordinateFitnessWeight(int id){
	return superordinateFitnessWeight[id];
}
void Theorist::calcBaseFitness(){
	for(int i=0;i<baseLength;i++){
		baseIndividual=baseGenotype[i];
		baseIndividualLength=baseMusicLength[i];
		baseIndividualChord=baseChord[i];
		for(int j=0;j<FITNESS_LENGTH;j++){
			baseDetailFitness[i][j]=(this->*fitnessForBaseFunctions[j])();
		}
	}
}
double Theorist::smooth(){
	double dif=0;
	int count=0;
	for(int i=0;i<musicLength-1;i++){
		int prevKey=individual[i][NOTE_KEY];
		int d=0;
		do{
			i++;
		}while(i<musicLength-1 && individual[i][NOTE_RISING]);
		count++;
		int presentKey=individual[i][NOTE_KEY];
		int higher=max(prevKey,presentKey);
		int lower=min(prevKey,presentKey);
		while(lower<higher){
			if(!pattern[lower%SCALE_LENGTH]){
				d++;
			}
			lower++;
		}
		if(d==0){
			d=3;
		}else if(d>COLOR_NOTES){
			d=COLOR_NOTES;
		}
		dif-=d;
		i--;
	}
	return dif/(count*COLOR_NOTES);
}
double Theorist::harmony(){
	double harmony=0;
	int count=0,totalCount=0;
	int disHarmonicTone=0;
	int nonHarmonicTone=0;
	int prevKey=0;
	for(int i=0;i<musicLength-1;i+=BASE_NUM){
		int nonHarmony=0;
		int disHarmony=0;
		int localCount=0;
		MusicChord *tmpChord=&chord[i/BASE_NUM];
		for(int j=0;i+j<musicLength && j<BASE_NUM;j++){
			if(!individual[i+j][NOTE_RISING]){
				int key=individual[i+j][NOTE_KEY];
				localCount++;
				if(pattern[key%SCALE_LENGTH]){
					disHarmony++;
					disHarmonicTone++;
				}else if(!chordManager.isIncludingTone(tmpChord->getRoot(),tmpChord->getQuality(),key%SCALE_LENGTH)){
					int nextKey;
					for(int k=1;i+j+k<musicLength && k<BASE_NUM;k++){
						if(!individual[i+j+k][NOTE_RISING]){
							nextKey=individual[i+j+k][NOTE_KEY];
							break;
						}
					}
					if(j==0){
						if(!chordManager.isIncludingTone(tmpChord->getRoot(),tmpChord->getQuality(),nextKey%SCALE_LENGTH) || calcInterval(key,nextKey)!=2){
							nonHarmony++;
						}
					}else if(j==BASE_NUM-1){
						MusicChord *nextChord=tmpChord+1;
						if(nextChord==NULL ||
							!chordManager.isIncludingTone(nextChord->getRoot(),nextChord->getQuality(),nextKey%SCALE_LENGTH)){
							nonHarmony++;
						}else if(key!=nextKey &&
							(calcInterval(prevKey,key)!=2 || calcInterval(key,nextKey)!=3 || calcInterval(prevKey,nextKey)>3)){
								nonHarmony++;
						}
					}else{
						if(!chordManager.isIncludingTone(tmpChord->getRoot(),tmpChord->getQuality(),prevKey%SCALE_LENGTH) ||
							!chordManager.isIncludingTone(tmpChord->getRoot(),tmpChord->getQuality(),nextKey%SCALE_LENGTH)){
							nonHarmony++;
						}else if(prevKey==nextKey){
							if(calcInterval(prevKey,key)!=2){
								nonHarmony++;
							}
						}else{
							if((nextKey-key)*(key-prevKey)<0){
								nonHarmony++;
							}
						}
					}
				}
				prevKey=key;
			}
		}
		nonHarmonicTone+=nonHarmony;
		if(localCount!=0 && (double)(nonHarmony+disHarmony)/localCount>0.35){
			//nonHarmonicTone++;
		}
		count++;
		totalCount+=localCount;
	}
	harmony=-(double)nonHarmonicTone/count;
	harmony=-(double)(nonHarmonicTone+disHarmonicTone*5)/totalCount;
	/*if((double)disHarmonicTone/totalCount>DISHARMONY_RATE){
		harmony=-1;
	}*/
	harmony=max(harmony,-1.0);
	return harmony;
}
double Theorist::rhythmicContinuity(){
	double dif=0;
	double prevDuration=0;
	for(int i=0;i<musicLength;i++){
		double count=0;
		do{
			count++;
			i++;
		}while(i<musicLength && individual[i][NOTE_RISING]);
		dif-=fabs(log(count+1)/log(2.0)-log(prevDuration+1)/log(2.0));
		prevDuration = count;
		i--;
	}
	dif = max(dif,-maxFitnesses[RHYTHMIC_CONTINUITY]);
	//return 0;
	return dif/maxFitnesses[RHYTHMIC_CONTINUITY];
}
double Theorist::standardDeviationOfMelody(){
	double sd=0;
	int counter[KEY_NUM];
	for(int i=0;i<KEY_NUM;i++){
		counter[i]=0;
	}
	for(int i=0;i<musicLength;i++){
		if(!individual[i][NOTE_RISING]){
			counter[individual[i][NOTE_KEY]]++;
		}
	}
	int index=0;
	int maxValue=counter[0];
	for(int i=1;i<KEY_NUM;i++){
		if(counter[i]>maxValue){
			index=i;
			maxValue=counter[i];
		}
	}
	for(int i=0;i<KEY_NUM;i++){
		if(counter[i]){
			sd+=counter[i]*pow(double(index-i),2);	
		}
	}
	sd=sqrt(sd/musicLength);
	sd=-fabs(sd-melodySD);
	sd=max(sd,-1.0);
	return sd;
}
double Theorist::chordAdaptation(){
	return chordManager.evaluateChords(chord,chordLength);
}
double Theorist::accentMelodyLine(){
	double accent=0;
	int count=0;
	double prevLowestAccent=KEY_NUM,prevHighestAccent=0,nextLowestAccent=KEY_NUM,nextHighestAccent=0;
	for(int i=0;i<musicLength;i+=BASE_NUM){
		int localCount=0;
		for(int j=0;i+j<musicLength && j<BASE_NUM;j++){
			if(!individual[i+j][NOTE_RISING]){
				if(prevHighestAccent==0){
					prevLowestAccent=min(prevLowestAccent,(double)individual[i+j][NOTE_KEY]);
					prevHighestAccent=max(prevHighestAccent,(double)individual[i+j][NOTE_KEY]);
				}else{
					nextLowestAccent=min(nextLowestAccent,(double)individual[i+j][NOTE_KEY]);
					nextHighestAccent=max(nextHighestAccent,(double)individual[i+j][NOTE_KEY]);
				}
				localCount++;
			}
		}
		if(nextHighestAccent!=0){
			if(nextLowestAccent-prevLowestAccent>=2 && nextHighestAccent-prevHighestAccent>=2){
				accent++;
			}
			count++;
			prevLowestAccent=KEY_NUM;
			nextLowestAccent=KEY_NUM;
			prevHighestAccent=0;
			nextHighestAccent=0;
		}
	}
	return -1+accent/count;
	/*double accent=0;
	int count=0;
	double prevAccent=0,nextAccent=0;
	for(int i=0;i<musicLength;i+=BASE_NUM*2){
		int localCount=0;
		for(int j=0;i+j<musicLength && j<BASE_NUM*2;j++){
			if(!individual[i+j][NOTE_RISING]){
				if(prevAccent==0){
					prevAccent+=individual[i+j][NOTE_KEY];
				}else{
					nextAccent+=individual[i+j][NOTE_KEY];
				}
				localCount++;
			}
		}
		if(nextAccent!=0){
			nextAccent/=localCount;
			if(nextAccent-prevAccent>=5){
				accent++;
			}
			count++;
			prevAccent=0;
			nextAccent=0;
		}else{
			prevAccent/=localCount;
		}
	}
	return -1+accent/count;*/
}
double Theorist::smoothForBase(){
	double dif=0;
	int count=0;
	for(int i=0;i<baseIndividualLength-1;i++){
		int prevKey=baseIndividual[i][NOTE_KEY];
		int d=0;
		do{
			i++;
		}while(i<baseIndividualLength-1 && baseIndividual[i][NOTE_RISING]);
		count++;
		int presentKey=baseIndividual[i][NOTE_KEY];
		int higher=max(prevKey,presentKey);
		int lower=min(prevKey,presentKey);
		while(lower<higher){
			if(!pattern[lower%SCALE_LENGTH]){
				d++;
			}
			lower++;
		}
		if(d==0){
			d=3;
		}else if(d>COLOR_NOTES){
			d=COLOR_NOTES;
		}else{
			
		}
		dif-=d;
		i--;
	}
	return 1+dif/(count*COLOR_NOTES);
}
double Theorist::harmonyForBase(){
	double harmony=0;
	int count=0,totalCount=0;
	int disHarmonicTone=0;
	int nonHarmonicTone=0;
	int prevKey=0;
	for(int i=0;i<baseIndividualLength-1;i+=BASE_NUM){
		int nonHarmony=0;
		int disHarmony=0;
		int localCount=0;
		MusicChord *tmpChord=&baseIndividualChord[i/BASE_NUM];
		for(int j=0;i+j<baseIndividualLength && j<BASE_NUM;j++){
			if(baseIndividual[i+j][NOTE_RISING]){
				int key=baseIndividual[i+j][NOTE_KEY];
				localCount++;
				if(pattern[key%SCALE_LENGTH]){
					disHarmony++;
					disHarmonicTone++;
				}else if(!chordManager.isIncludingTone(tmpChord->getRoot(),tmpChord->getQuality(),key%SCALE_LENGTH)){
					int nextKey;
					for(int k=1;i+j+k<baseIndividualLength && k<BASE_NUM;k++){
						if(!baseIndividual[i+j+k][NOTE_RISING]){
							nextKey=baseIndividual[i+j+k][NOTE_KEY];
							break;
						}
					}
					if(j==0){
						if(!chordManager.isIncludingTone(tmpChord->getRoot(),tmpChord->getQuality(),nextKey%SCALE_LENGTH) || calcInterval(key,nextKey)!=2){
							nonHarmony++;
						}
					}else if(j==BASE_NUM-1){
						MusicChord *nextChord=tmpChord+1;
						if(nextChord==NULL ||
							!chordManager.isIncludingTone(nextChord->getRoot(),nextChord->getQuality(),nextKey%SCALE_LENGTH)){
							nonHarmony++;
						}else if(key!=nextKey &&
							(calcInterval(prevKey,key)!=2 || calcInterval(key,nextKey)!=3 || calcInterval(prevKey,nextKey)>3)){
								nonHarmony++;
						}
					}else{
						if(!chordManager.isIncludingTone(tmpChord->getRoot(),tmpChord->getQuality(),prevKey%SCALE_LENGTH) ||
							!chordManager.isIncludingTone(tmpChord->getRoot(),tmpChord->getQuality(),nextKey%SCALE_LENGTH)){
							nonHarmony++;
						}else if(prevKey==nextKey){
							if(calcInterval(prevKey,key)!=2){
								nonHarmony++;
							}
						}else{
							if((nextKey-key)*(key-prevKey)<0){
								nonHarmony++;
							}
						}
					}
				}
				prevKey=key;
			}
		}
		nonHarmonicTone+=nonHarmony;
		if(localCount!=0 && (double)(nonHarmony)/localCount>0.35){
			//nonHarmonicTone++;
		}
		count++;
		totalCount+=localCount;
	}
	harmony=-(double)nonHarmonicTone/count;
	harmony=-(double)(nonHarmonicTone+disHarmonicTone)/totalCount;
	/*if((double)disHarmonicTone/totalCount>DISHARMONY_RATE){
		harmony=-1;
	}*/
	//return 0;
	return 1+harmony;
}
double Theorist::rhythmicContinuityForBase(){
	double dif=0;
	double prevDuration=0;
	for(int i=0;i<baseIndividualLength;i++){
		double count=0;
		do{
			count++;
			i++;
		}while(i<baseIndividualLength && baseIndividual[i][NOTE_RISING]);
		//dif-=log((fabs(count-prevDuration))+1)/log(2.0);
		dif-=fabs(log(count+1)/log(2.0)-log(prevDuration+1)/log(2.0));
		prevDuration = count;
		i--;
	}
	dif = max(dif,-maxFitnesses[RHYTHMIC_CONTINUITY]);
	//return -1;
	return 1+dif/maxFitnesses[RHYTHMIC_CONTINUITY];
}
double Theorist::standardDeviationOfMelodyForBase(){
	double sd=0;
	int counter[KEY_NUM];
	for(int i=0;i<KEY_NUM;i++){
		counter[i]=0;
	}
	for(int i=0;i<baseIndividualLength;i++){
		if(!baseIndividual[i][NOTE_RISING]){
			counter[baseIndividual[i][NOTE_KEY]]++;
		}
	}
	int index=0;
	int maxValue=counter[0];
	for(int i=1;i<KEY_NUM;i++){
		if(counter[i]>maxValue){
			index=i;
			maxValue=counter[i];
		}
	}
	for(int i=0;i<KEY_NUM;i++){
		if(counter[i]){
			sd+=counter[i]*pow(double(index-i),2);	
		}
	}
	sd=sqrt(sd/baseIndividualLength);
	sd=-fabs(sd-melodySD);
	if(sd<-1){
		sd=-1;
	}
	sd=max(sd,-1.0);
	return 1+sd;
}
double Theorist::accentMelodyLineForBase(){
	double accent=0;
	int count=0;
	double prevLowestAccent=KEY_NUM,prevHighestAccent=0,nextLowestAccent=KEY_NUM,nextHighestAccent=0;
	for(int i=0;i<baseIndividualLength;i+=BASE_NUM*2){
		int localCount=0;
		for(int j=0;i+j<baseIndividualLength && j<BASE_NUM*2;j++){
			if(!baseIndividual[i+j][NOTE_RISING]){
				if(prevHighestAccent==0){
					prevLowestAccent=min(prevLowestAccent,(double)baseIndividual[i+j][NOTE_KEY]);
					prevHighestAccent=max(prevHighestAccent,(double)baseIndividual[i+j][NOTE_KEY]);
				}else{
					nextLowestAccent=min(nextLowestAccent,(double)baseIndividual[i+j][NOTE_KEY]);
					nextHighestAccent=max(nextHighestAccent,(double)baseIndividual[i+j][NOTE_KEY]);
				}
				localCount++;
			}
		}
		if(nextHighestAccent!=0){
			if(nextLowestAccent-prevLowestAccent>=2 && nextHighestAccent-prevHighestAccent>=2){
				accent++;
			}
			count++;
			prevLowestAccent=KEY_NUM;
			nextLowestAccent=KEY_NUM;
			prevHighestAccent=0;
			nextHighestAccent=0;
		}
	}
	return accent/count;
}
double Theorist::chordAdaptationForBase(){
	return 1+chordManager.evaluateChords(baseIndividualChord,baseIndividualLength/BASE_NUM);
}
double Theorist::returnZero(){
	return 0;
}
double Theorist::returnMinus(){
	return MINUS;
}
double Theorist::returnPlus(){
	return PLUS;
}
void Theorist::setStandardDeviationOfMelodyForBase(){
	melodySD=0;
	int counter[KEY_NUM];
	for(int n=0;n<baseLength;n++){
		double sd=0;
		for(int i=0;i<KEY_NUM;i++){
			counter[i]=0;
		}
		for(int i=0;i<baseMusicLength[n];i++){
			if(!baseGenotype[n][i][NOTE_RISING]){
				counter[baseGenotype[n][i][NOTE_KEY]]++;
			}
		}
		int index=0;
		int max=counter[0];
		for(int i=1;i<KEY_NUM;i++){
			if(counter[i]>max){
				index=i;
				max=counter[i];
			}
		}
		for(int i=0;i<KEY_NUM;i++){
			if(counter[i]){
				sd+=counter[i]*pow(double(index-i),2);
			}
		}
		sd=sqrt(sd/baseMusicLength[n]);
		melodySD+=sd;
	}
	melodySD/=baseLength;
	//melodySD=SCALE_LENGTH/2;
	//melodySD=2;

	/*
	//tmp
		melodySD=random.nextDouble(5)+5;
	//tmp
	*/
	cout << "Melody SD: " << melodySD << endl;
}
void Theorist::setImpulseRate(){
	int count=0;
	int nonZeroCount=0;
	/*for(int i=0;i<baseMusicLength[0];i++){
		if(!baseGenotype[0][i][NOTE_RISING]){
			impulseRate[i%BASE_NUM]++;
			count++;
		}
	}*/
	for(int i=0;i<baseLength;i++){
		for(int j=0;j<baseMusicLength[i];j++){
			if(!baseGenotype[i][j][NOTE_RISING]){
				impulseRate[j%BASE_NUM]++;
				count++;
			}
		}
	}

	/*
	//tmp
		for(int i=0;i<BASE_NUM;i++){
			impulseRate[i]=0;
		}
		for(int i=0;i<128;i++){
			int tmp=random.nextInt(2);
			impulseRate[i%BASE_NUM]+=tmp;
			if(tmp){
				count++;
			}
		}
	//tmp
	*/

	double sum=0;
	maxImpulseRate=0;
	averageImpulseRate=0;
	overAverageImpulseRate=0;
	for(int i=0;i<BASE_NUM;i++){
		impulseRate[i]/=count;
		if(maxImpulseRate<impulseRate[i]){
			maxImpulseRate=impulseRate[i];
		}
		cout << "impulseRate" << i << ": " << impulseRate[i] << endl;
		if(impulseRate[i]!=0){
			sum+=impulseRate[i];
			nonZeroCount++;
		}
	}
	averageImpulseRate=sum/nonZeroCount;
	//averageImpulseRate=sum/BASE_NUM;
	cout << "average impulseRate" << ": " << averageImpulseRate << endl;
	for(int i=0;i<BASE_NUM;i++){
		if(impulseRate[i]>=averageImpulseRate){
			overAverageImpulseRate+=impulseRate[i];
		}
	}
	cout << "over-average impulseRate" << ": " << overAverageImpulseRate << endl;
}
void Theorist::setBaseChord(){
	chordManager.setScale(scale);
	baseChord = new MusicChord*[baseLength];
	for(int i=0;i<baseLength;i++){
		chordManager.setNotes(baseGenotype[i]);
		chordManager.setMusicLength(baseMusicLength[i]);
		baseChord[i] = chordManager.judgeChord();
	}
}
void Theorist::setFitnessWeight(){
	double sum=0;
	int count=0;
	/*for(int i=0;i<baseLength;i++){
		for(int j=0;j<FITNESS_LENGTH;j++){
			fitnessWeight[j]+=baseDetailFitness[i][j];
		}
	}
	for(int i=0;i<FITNESS_LENGTH;i++){
		sum+=fitnessWeight[i];
	}
	for(int i=0;i<FITNESS_LENGTH;i++){
		fitnessWeight[i]=fitnessWeight[i]/sum;
	}*/

	for(int i=0;i<baseLength;i++){
		for(int j=0;j<FITNESS_LENGTH;j++){
			if(baseDetailFitness[i][j]!=PLUS && baseDetailFitness[i][j]!=MINUS){
				fitnessWeight[j]+=baseDetailFitness[i][j];
				sum+=baseDetailFitness[i][j];
				count++;
			}
		}
	}
	double average=sum/count;
	for(int i=0;i<baseLength;i++){
		for(int j=0;j<FITNESS_LENGTH;j++){
			if(baseDetailFitness[i][j]==PLUS){
				fitnessWeight[j]+=average;
			}
		}
	}
	sum=0;
	for(int i=0;i<FITNESS_LENGTH;i++){
		cout << fitnessWeight[i] << endl;
		sum+=fitnessWeight[i];
	}
	for(int i=0;i<FITNESS_LENGTH;i++){
		fitnessWeight[i]=fitnessWeight[i]/sum;
	}

	/*
	//tmp
		for(int i=0;i<FITNESS_LENGTH;i++){
			if(i==FITNESS_LENGTH-1){
				fitnessWeight[i]=0;
			}else{
				fitnessWeight[i]=1.0/(FITNESS_LENGTH-1);
			}
		}
	//tmp
	*/
}
void Theorist::printFitnessWeight(){
	cout << "---Fitness Weight---" << endl;
	for(int i=0;i<FITNESS_LENGTH;i++){
		cout << FITNESS_NAME[i] << ": " << fitnessWeight[i] << endl;
	}
}
void Theorist::printBaseFitness(){
	cout << "---Base Fitnesses---" << endl;
	for(int i=0;i<baseLength;i++){
		double fitness=0;
		cout << "ID" << i << ":" << endl;
		for(int j=0;j<FITNESS_LENGTH;j++){
			//double detailFitness=fitnessWeight[j]*baseDetailFitness[i][j];
			double detailFitness=baseDetailFitness[i][j];
			cout << FITNESS_NAME[j] << ": " << flush;
			if(detailFitness!=PLUS){
				cout << detailFitness << endl;
				fitness+=detailFitness;
			}else{
				cout << "-" << endl;
			}
		}
		cout << "Total Fitness " << fitness << endl;
	}
}
double* Theorist::getFitnessWeight(){
	return fitnessWeight;
}
double Theorist::getPitchRange(){
	return melodySD;
}
double* Theorist::getImpulseRate(){
	return impulseRate;
}
double Theorist::getAverageImpulseRate(){
	return averageImpulseRate;
}
double Theorist::getOverAverageImpulseRate(){
	return overAverageImpulseRate;
}
int Theorist::calcInterval(int prev,int next){
	int interval=1,start,end;
	start=min(prev,next);
	end=max(prev,next);
	for(int i=start+1;i<=end;i++){
		if(!pattern[start%SCALE_LENGTH]){
			interval++;
		}
	}
	return interval;
}