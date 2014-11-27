#include "Composer.h"

Composer::Composer(void)
{
}
Composer::~Composer(void)
{
}
void Composer::modulateMusic(Music *music,int length){
	int baseTonic = C;
	int baseScale = MAJOR;
	for(int i=0;i<length;i++){
		if(baseScale==music[i].getScale()){
			music[i].shiftKey(baseTonic-music[i].getTonic());
		}else if(baseScale && !music[i].getScale()){
			music[i].shiftKey(baseTonic+9-music[i].getTonic());
		}else{
			music[i].shiftKey(baseTonic+3-music[i].getTonic());
		}
	}
}
void Composer::mixByGA(Music *music,int length){
	int *musicLength = new int[length];
	int ***base = new int**[length];
	int maxLength=0;
	//int randomLength=1000;
	for(int i=0;i<length;i++){
		musicLength[i] = music[i].getBasicNoteLength();
		maxLength = max(maxLength,musicLength[i]);
		//musicLength[i] = randomLength;
		//maxLength = randomLength;
	}
	/*for(int i=0;i<length;i++){
		base[i] = new int*[maxLength];
		for(int j=0;j<maxLength;j++){
			base[i][j] = new int[NOTE_CONTENT];
		}
	}*/
	tonic = C;
	scale = MAJOR;
	modulateMusic(music,length);
	for(int i=0;i<length;i++){
		base[i] = music[i].encodeNote(maxLength);
		//base[i]=music[i].generateRandomMusic(randomLength);
	}
	ga.setBase(base,musicLength,length,scale);
	ga.setGA();
	int **test = ga.getBestIndividual();	
	preMix.setScale(scale);
	preMix.setTonic(tonic);
	preMix.decodeNote(test,ga.getMusicLength());
	//preMix.shiftKey(tonic);
	ga.startGA();
	test = ga.getBestIndividual();
	mixed.setScale(scale);
	mixed.setTonic(tonic);
	mixed.decodeNote(test,ga.getMusicLength());
	//mixed.shiftKey(tonic);
	mixed.judgeTonality();
	setProperties();
	ga.deleteGA();
}
Music* Composer::getPreMix(){
	return &preMix;
}
Music* Composer::getMixed(){
	return &mixed;
}
void Composer::setProperties(){
	propertiesManager->setGAParameters(ga.getPopSize(),ga.getMusicLength(),ga.getMaxGeneration(),ga.getCrossoverRate(),ga.getMutationRate(),ga.getSelfCopyRate());
	propertiesManager->setFitnessWeight(ga.getFitnessWeight());
	propertiesManager->setEliteGeneration(ga.getEliteGeneration());
	propertiesManager->setEliteDetailFitness(ga.getEliteDetailFitness());
	propertiesManager->setEliteDetailMusicTheory(ga.getEliteDetailMusicTheory());
	propertiesManager->setEliteDetailGlobalSimilarity(ga.getEliteDetailGlobalSimilarity());
	propertiesManager->setLocalSimilarity(ga.getLocalSimilarity());
	propertiesManager->setPitchRange(ga.getPitchRange());
	propertiesManager->setImpulseRate(ga.getImpulseRate());
	propertiesManager->setAverageImpulseRate(ga.getAverageImpulseRate());
	propertiesManager->setOverAverageImpulseRate(ga.getOverAverageImpulseRate());
}
void Composer::setPropertiesManager(PropertiesManager *manager){
	propertiesManager=manager;
}