#include "MusicChord.h"
#include"ChordManager.h"

MusicChord::MusicChord(KEY key,QUALITY q,ADDITIVE_QUALITY aQ,int oc)
{
	root=key;
	quality=q;
	additiveQuality=aQ;
	octave=oc;
}
MusicChord::~MusicChord(void)
{
}
void MusicChord::setChord(KEY key,QUALITY q,int oc){
	root=key;
	quality=q;
	octave=oc;
}
void MusicChord::setAdditiveQuality(ADDITIVE_QUALITY aQ){
	additiveQuality=aQ;
	additiveQuality=SIMPLE;
}
int *MusicChord::getTones(){
	ChordManager chordManager;
	int *tones = new int[QUALITY_COMPOSITION_NUM+CHORD_ADDITION_LENGTH];
	int *t = chordManager.getChordTones(quality);
	for(int i=0;i<QUALITY_COMPOSITION_NUM;i++){
		if(t[i]!=-1){
			tones[i]=(t[i]+(int)root)%SCALE_LENGTH;
		}else{
			tones[i]=-1;
		}
	}
	t=chordManager.getAdditiveChordTones(additiveQuality);
	for(int i=0;i<CHORD_ADDITION_LENGTH;i++){
		if(t[i]!=-1){
			tones[i+QUALITY_COMPOSITION_NUM]=(t[i]+(int)root)%SCALE_LENGTH;
		}else{
			tones[i+QUALITY_COMPOSITION_NUM]=-1;
		}
	}
	return tones;
}
void MusicChord::setToneNum(int num){
	toneNum=num;
}
KEY MusicChord::getRoot(){
	return root;
}
QUALITY MusicChord::getQuality(){
	return quality;
}
ADDITIVE_QUALITY MusicChord::getAdditiveQuality(){
	return additiveQuality;
}
int MusicChord::getToneNum(){
	return toneNum;
}
void MusicChord::printChord(){
	if(root != UNDEFINED_KEY){
		cout << keyName[root] << qualityName[quality] << additiveQualityName[additiveQuality] << endl;
	}
}
string MusicChord::getChordName(){
	string name = "";
	name+=keyName[root];
	name+=qualityName[quality];
	name+=additiveQualityName[additiveQuality];
	
	return name;
}
void MusicChord::setNote(Note *note){
	this->note=note;
}
Note* MusicChord::getNote(){
	return note;
}
Note MusicChord::getNote(int index){
	return note[index];
}