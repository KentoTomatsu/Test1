#include "PropertiesManager.h"
#include "FileManager.h"

PropertiesManager::PropertiesManager(void)
{
}
PropertiesManager::~PropertiesManager(void)
{
}
void PropertiesManager::setBaseMusicName(string *name){
	baseMusicName=name;
}
void PropertiesManager::setBaseLength(int length){
	baseLength=length;
}
void PropertiesManager::setGAParameters(int popSize,int geneSize,int maxGeneration,double crossoverRate,double mutationRate,double selfCopyRate){
	this->popSize=popSize;
	this->geneSize=geneSize;
	this->maxGeneration=maxGeneration;
	this->crossoverRate=crossoverRate;
	this->mutationRate=mutationRate;
	this->selfCopyRate=selfCopyRate;
}
void PropertiesManager::setFitnessWeight(double *fitnessWeight){
	for(int i=0;i<FITNESS_LENGTH;i++){
		this->fitnessWeight[i]=fitnessWeight[i];
	}
}
void PropertiesManager::setEliteGeneration(int generation){
	eliteGeneration=generation;
}
void PropertiesManager::setEliteDetailFitness(double *detailFitness){
	for(int i=0;i<SUPERORDINATE_FITNESS_LENGTH;i++){
		eliteDetailFitness[i]=detailFitness[i];
	}
	eliteFitness=0;
	for(int i=0;i<SUPERORDINATE_FITNESS_LENGTH;i++){
		eliteFitness+=eliteDetailFitness[i];
	}
}
void PropertiesManager::setEliteDetailMusicTheory(double *detailMusicTheory){
	for(int i=0;i<FITNESS_LENGTH;i++){
		eliteDetailMusicTheory[i]=detailMusicTheory[i];
	}
}
void PropertiesManager::setEliteDetailGlobalSimilarity(double *detailGlobalSimilarity){
	for(int i=0;i<GLOBAL_SIMILARITY_LENGTH;i++){
		eliteDetailGlobalSimilarity[i]=detailGlobalSimilarity[i];
	}
}
void PropertiesManager::setLocalSimilarity(double localSimilarity){
	this->localSimilarity=localSimilarity;
}
void PropertiesManager::setTime(double time){
	this->time=time;
}
void PropertiesManager::setKeyFreq(){

}
void PropertiesManager::setPitchRange(double range){
	pitchRange=range;
}
void PropertiesManager::setImpulseRate(double *impulse){
	for(int i=0;i<BASE_NUM;i++){
		impulseRate[i]=impulse[i];
	}
}
void PropertiesManager::setAverageImpulseRate(double impulse){
	averageImpulseRate=impulse;
}
void PropertiesManager::setOverAverageImpulseRate(double impulse){
	overAverageImpulseRate=impulse;
}
void PropertiesManager::saveProperties(){
	FileManager fileManager;
	fileManager.openStream(FileManager::getFileName(PROPERTIES).c_str());
	fileManager.saveProperties(createStringData());

}
string PropertiesManager::createStringData(){
	string data="";
	data.append(createString("Base Length",baseLength));
	for(int i=0;i<baseLength;i++){
		data.append(baseMusicName[i]+"\n");
	}
	data.append(createString("Population Size",popSize));
	data.append(createString("Gene Size",geneSize));
	data.append(createString("Max Generation",maxGeneration));
	data.append(createString("Crossover Rate",crossoverRate));
	data.append(createString("Mutation Rate",mutationRate));
	data.append(createString("Self-Copy Rate",selfCopyRate));
	data.append("Fitness Weight\n");
	for(int i=0;i<FITNESS_LENGTH;i++){
		data.append("\t");
		data.append(createString(FITNESS_NAME[i],fitnessWeight[i]));
	}
	data.append("Best Individual\n");
	data.append(createString("Generation",eliteGeneration));
	data.append(createString("Total Fitness",eliteFitness));
	data.append(createString("Music Theory",eliteDetailFitness[MUSIC_THEORY]));
	for(int i=0;i<FITNESS_LENGTH;i++){
		data.append("\t");
		data.append(createString(FITNESS_NAME[i],eliteDetailMusicTheory[i]));
	}
	data.append(createString("Global Similarity",eliteDetailFitness[GLOBAL]));
	for(int i=0;i<GLOBAL_SIMILARITY_LENGTH;i++){
		data.append("\t");
		data.append(createString(GLOBAL_SIMILARITY_NAME[i] ,eliteDetailGlobalSimilarity[i]));
	}
	data.append(createString("Local Similarity",localSimilarity));
	data.append(createString("Time",time));
	data.append(createString("Pitch Range",pitchRange));
	data.append("Impulse Rate\n");
	for(int i=0;i<BASE_NUM;i++){
		data.append(createString("impulse",impulseRate[i]));
	}
	data.append(createString("impulse Ave.",averageImpulseRate));
	data.append(createString("impulse OverAve.",overAverageImpulseRate));

	return data;
}
