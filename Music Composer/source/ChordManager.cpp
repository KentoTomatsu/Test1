#include "ChordManager.h"

ChordManager::ChordManager(void)
{
	setChordCompositon();
	setChordAdaptation();
	setChordContinuity();
}
ChordManager::~ChordManager(void)
{
}
void ChordManager::setChordCompositon(){
	chordComposition[MAJ][0]=0;
	chordComposition[MAJ][1]=4;
	chordComposition[MAJ][2]=7;
	chordComposition[MIN][0]=0;
	chordComposition[MIN][1]=3;
	chordComposition[MIN][2]=7;

	chordAddition[SIMPLE][0] = -1;
	chordAddition[SIMPLE][1] = -1;
	chordAddition[ADDED_NINETH][0] = 2;
	chordAddition[ADDED_NINETH][1] = -1;
	chordAddition[SIXTH][0] = 9;
	chordAddition[SIXTH][1] = -1;
	chordAddition[SIXTH_ADD_NINE][0] = 2;
	chordAddition[SIXTH_ADD_NINE][1] = 9;
	chordAddition[SEVENTH][0] = 10;
	chordAddition[SEVENTH][1] = -1;
	chordAddition[MAJOR_SEVENTH][0] = 11;
	chordAddition[MAJOR_SEVENTH][1] = -1;
	chordAddition[SEVENTH_NINETH][0] = 10;
	chordAddition[SEVENTH_NINETH][1] = 2;
	chordAddition[MAJOR_SEVENTH_NINETH][0] = 11;
	chordAddition[MAJOR_SEVENTH_NINETH][1] = 2;
}

void ChordManager::setChordAdaptation(){
	chordAdaptation[C][MAJ]=1;
	chordAdaptation[Cs][MAJ]=MIN_CHORD_ADAPTATION;
	chordAdaptation[D][MAJ]=0.8;
	chordAdaptation[Ds][MAJ]=0.4;
	chordAdaptation[E][MAJ]=0.6;
	chordAdaptation[F][MAJ]=1;
	chordAdaptation[Fs][MAJ]=MIN_CHORD_ADAPTATION;
	chordAdaptation[G][MAJ]=1;
	chordAdaptation[Gs][MAJ]=0.5;
	chordAdaptation[A][MAJ]=MIN_CHORD_ADAPTATION;
	chordAdaptation[As][MAJ]=0.5;
	chordAdaptation[B][MAJ]=MIN_CHORD_ADAPTATION;

	chordAdaptation[C][MIN]=MIN_CHORD_ADAPTATION;
	chordAdaptation[Cs][MIN]=MIN_CHORD_ADAPTATION;
	chordAdaptation[D][MIN]=1;
	chordAdaptation[Ds][MIN]=MIN_CHORD_ADAPTATION;
	chordAdaptation[E][MIN]=1;
	chordAdaptation[F][MIN]=0.8;
	chordAdaptation[Fs][MIN]=MIN_CHORD_ADAPTATION;
	chordAdaptation[G][MIN]=MIN_CHORD_ADAPTATION;
	chordAdaptation[Gs][MIN]=MIN_CHORD_ADAPTATION;
	chordAdaptation[A][MIN]=1;
	chordAdaptation[As][MIN]=MIN_CHORD_ADAPTATION;
	chordAdaptation[B][MIN]=1;
}

void ChordManager::setChordContinuity(){
	for(int i=0;i<SCALE_LENGTH;i++){
		for(int j=0;j<QUALITY_LENGTH;j++){
			for(int k=0;k<SCALE_LENGTH;k++){
				for(int l=0;l<QUALITY_LENGTH;l++){
					chordContinuity[i][j][k][l]=MIN_CHORD_CONTINUITY;
				}
			}
		}
	}
	
	chordContinuity[C][MAJ][D][MIN]=1;
	chordContinuity[C][MAJ][E][MAJ]=0.7;
	chordContinuity[C][MAJ][E][MIN]=1;
	chordContinuity[C][MAJ][F][MAJ]=1;
	chordContinuity[C][MAJ][F][MIN]=0.6;
	chordContinuity[C][MAJ][G][MAJ]=1;
	chordContinuity[C][MAJ][G][MIN]=0.5;
	chordContinuity[C][MAJ][A][MAJ]=0.4;
	chordContinuity[C][MAJ][A][MIN]=1;
	chordContinuity[C][MAJ][B][MIN]=0.4;

	/*chordContinuity[D][MAJ][C][MAJ]=0.4;
	chordContinuity[D][MAJ][D][MIN]=0.5;
	chordContinuity[D][MAJ][E][MAJ]=0.6;
	chordContinuity[D][MAJ][E][MIN]=0.5;*/
	chordContinuity[D][MAJ][F][MAJ]=0.8;
	chordContinuity[D][MAJ][G][MAJ]=1;
	//chordContinuity[D][MAJ][A][MIN]=0.5;

	chordContinuity[D][MIN][C][MAJ]=0.3;
	chordContinuity[D][MIN][E][MAJ]=0.4;
	chordContinuity[D][MIN][E][MIN]=0.6;
	chordContinuity[D][MIN][F][MIN]=0.3;
	chordContinuity[D][MIN][G][MAJ]=1;
	chordContinuity[D][MIN][G][MIN]=0.1;
	chordContinuity[D][MIN][A][MAJ]=0.1;
	chordContinuity[D][MIN][A][MIN]=0.3;

	chordContinuity[E][MAJ][C][MAJ]=0.8;
	chordContinuity[E][MAJ][D][MIN]=0.6;
	chordContinuity[E][MAJ][E][MIN]=0.3;
	chordContinuity[E][MAJ][F][MAJ]=0.8;
	chordContinuity[E][MAJ][F][MIN]=0.3;
	chordContinuity[E][MAJ][G][MAJ]=0.5;
	chordContinuity[E][MAJ][G][MIN]=0.1;
	chordContinuity[E][MAJ][A][MAJ]=0.4;
	chordContinuity[E][MAJ][A][MIN]=1;

	chordContinuity[E][MIN][C][MAJ]=0.5;
	chordContinuity[E][MIN][D][MIN]=0.6;
	chordContinuity[E][MIN][E][MAJ]=0.6;
	chordContinuity[E][MIN][F][MAJ]=0.8;
	chordContinuity[E][MIN][F][MIN]=0.5;
	chordContinuity[E][MIN][G][MIN]=0.3;
	chordContinuity[E][MIN][A][MAJ]=0.5;
	chordContinuity[E][MIN][A][MIN]=1;

	chordContinuity[F][MAJ][C][MAJ]=0.9;
	chordContinuity[F][MAJ][D][MIN]=0.5;
	chordContinuity[F][MAJ][E][MAJ]=0.7;
	chordContinuity[F][MAJ][E][MIN]=0.5;
	chordContinuity[F][MAJ][F][MIN]=0.8;
	chordContinuity[F][MAJ][G][MAJ]=1;
	chordContinuity[F][MAJ][A][MAJ]=0.5;
	chordContinuity[F][MAJ][A][MIN]=0.7;

	chordContinuity[F][MIN][C][MAJ]=0.6;
	chordContinuity[F][MIN][D][MIN]=0.4;
	chordContinuity[F][MIN][E][MAJ]=0.7;
	chordContinuity[F][MIN][E][MIN]=1;
	chordContinuity[F][MIN][F][MAJ]=0.5;
	chordContinuity[F][MIN][G][MAJ]=0.9;
	chordContinuity[F][MIN][G][MIN]=0.3;
	chordContinuity[F][MIN][A][MAJ]=0.4;
	chordContinuity[F][MIN][A][MIN]=0.8;

	chordContinuity[G][MAJ][C][MAJ]=1;
	chordContinuity[G][MAJ][D][MIN]=0.4;
	chordContinuity[G][MAJ][E][MAJ]=0.6;
	chordContinuity[G][MAJ][E][MIN]=0.8;
	chordContinuity[G][MAJ][F][MAJ]=0.5;
	chordContinuity[G][MAJ][F][MIN]=0.1;
	chordContinuity[G][MAJ][A][MAJ]=0.4;
	chordContinuity[G][MAJ][A][MIN]=0.5;

	chordContinuity[G][MIN][C][MAJ]=0.9;
	chordContinuity[G][MIN][D][MIN]=0.4;
	chordContinuity[G][MIN][E][MAJ]=0.7;
	chordContinuity[G][MIN][E][MIN]=1;
	chordContinuity[G][MIN][F][MAJ]=0.9;
	chordContinuity[G][MIN][F][MIN]=0.7;
	chordContinuity[G][MIN][A][MAJ]=0.4;
	chordContinuity[G][MIN][A][MIN]=0.8;

	chordContinuity[A][MAJ][C][MAJ]=0.8;
	chordContinuity[A][MAJ][D][MIN]=0.8;
	chordContinuity[A][MAJ][E][MAJ]=0.4;
	chordContinuity[A][MAJ][E][MIN]=0.5;
	chordContinuity[A][MAJ][F][MAJ]=0.9;
	chordContinuity[A][MAJ][F][MIN]=0.7;
	chordContinuity[A][MAJ][G][MAJ]=0.7;
	chordContinuity[A][MAJ][A][MIN]=0.7;

	chordContinuity[A][MIN][D][MIN]=0.4;
	chordContinuity[A][MIN][E][MAJ]=0.7;
	chordContinuity[A][MIN][E][MIN]=0.6;
	chordContinuity[A][MIN][F][MAJ]=1;
	chordContinuity[A][MIN][F][MIN]=0.7;
	chordContinuity[A][MIN][G][MAJ]=0.8;
	chordContinuity[A][MIN][G][MIN]=0.4;

}
void ChordManager::setChord(MusicChord *c){
	//chord=c;
}
void ChordManager::setQuality(){
	
}
int ChordManager::getQualityToneNum(QUALITY q){
	switch(q){
	case MAJ:
		return 3;
	case MIN:
		return 3;
	default:
		return 0;
	}
}
void ChordManager::printChord(MusicChord *chord,int length){
	for(int i=0;i<length;i++){
		cout << keyName[(int)chord[i].getRoot()] << qualityName[(int)chord[i].getQuality()] << " ";
	}
}
void ChordManager::setNotes(int **notes){
	this->notes = notes;
}
void ChordManager::setMusicLength(int length){
	musicLength=length;
}
void ChordManager::setScale(int scale){
	this->scale=scale;
}
MusicChord* ChordManager::judgeChord(){
	int tones[BASE_NUM];
	MusicChord *chord = new MusicChord[musicLength/BASE_NUM+1];
	prevChord.setChord(UNDEFINED_KEY,UNDEFINED_QUALITY,0);
	for(int i=0;i<musicLength;i+=BASE_NUM){
		int index=i;
		for(int k=0;k<BASE_NUM;k++){
			tones[k]=-1;
		}
		for(int j=0;j<BASE_NUM && index<musicLength;j++,index++){
			if(notes[index][NOTE_RISING]==0){
				tones[j]=notes[index][NOTE_KEY];
			}else{
				tones[j]=-1;
			}
		}
		chord[i/BASE_NUM] = judgeSingleChord(tones);
		chord[i/BASE_NUM].setAdditiveQuality(setAdditiveQuality(chord->getRoot(),chord->getQuality(),tones));
		prevChord = chord[i/BASE_NUM];
	}
	return chord;
}
MusicChord ChordManager::judgeSingleChord(int *tones){
	int noteCount[SCALE_LENGTH];
	for(int i=0;i<SCALE_LENGTH;i++){
		noteCount[i]=0;
	}
	for(int i=0;i<BASE_NUM;i++){
		if(tones[i]!=-1){
			noteCount[tones[i]%SCALE_LENGTH]++;
		}
	}
	return getMatchChord(noteCount);
}
MusicChord ChordManager::getMatchChord(int *noteCount){
	MusicChord chord;
	int root=0;
	int quality=0;
	int max=0;
	for(int i=0;i<SCALE_LENGTH;i++){
		for(int j=0;j<QUALITY_LENGTH;j++){
			double match=0;
			int sum=0;
			for(int k=0;k<SCALE_LENGTH;k++){
				if(isIncludingTone(i,j,k)){
					match+=noteCount[k];
				}
				sum+=noteCount[k];
			}
			match=match/sum*100;
			chordFitness[i][j]=match*getChordAdaptation((KEY)i,(QUALITY)j)*getChordContinuity((KEY)i,(QUALITY)j);
			//chordFitness[i][j]=chordFitness[i][j]*(random.nextDouble(0.5)+0.5);
			if(max<chordFitness[i][j]){
				max=chordFitness[i][j];
				root=i;
				quality=j;
			}
		}
	}
	chord.setChord((KEY)root,(QUALITY)quality,4);
	return chord;
}
bool ChordManager::isIncludingTone(int root,int quality,int key){
	for(int i=0;i<QUALITY_COMPOSITION_NUM;i++){
		if(chordComposition[quality][i]==-1){
			return false;
		}
		int dif=key-root;
		if(dif<0){
			dif+=SCALE_LENGTH;
		}
		if(dif==chordComposition[quality][i]){
			return true;
		}
	}
	return false;
}
ADDITIVE_QUALITY ChordManager::setAdditiveQuality(KEY root,QUALITY quality,int *tones){
	double noteCount[SCALE_LENGTH];
	int count=0;
	double simple = 0;
	for(int i=0;i<SCALE_LENGTH;i++){
		noteCount[i]=0;
	}
	for(int i=0;i<BASE_NUM;i++){
		if(tones[i]!=-1){
			noteCount[tones[i]%SCALE_LENGTH]++;
			count++;
		}
	}
	for(int i=0;i<QUALITY_COMPOSITION_NUM;i++){
		noteCount[(root+chordComposition[quality][i])%SCALE_LENGTH]=0;
	}
	for(int i=0;i<SCALE_LENGTH;i++){
		noteCount[i]/=count;
		simple+=noteCount[i];
	}
	simple=1-simple;
	double max=0;
	int index=0;
	for(int i=0;i<ADDITIVE_QUALITY_LENGTH;i++){
		double match=0;
		for(int j=0;j<SCALE_LENGTH;j++){
			if(isIncludingAdditiveTone(root,i,j)){
				match+=noteCount[j];
			}
		}
		if(i==SIMPLE){
			match=simple;
		}
		//match=match*(random.nextDouble(0.5)+0.5);
		if(max<match){
			max=match;
			index=i;
		}
	}
	return ADDITIVE_QUALITY(index);
}
bool ChordManager::isIncludingAdditiveTone(int root,int quality,int key){
	for(int i=0;i<CHORD_ADDITION_LENGTH;i++){
		if(chordAddition[quality][i]==-1){
			return false;
		}
		int dif=key-root;
		if(dif<0){
			dif+=SCALE_LENGTH;
		}
		if(dif==chordAddition[quality][i]){
			return true;
		}
	}
	return false;
}
double ChordManager::getChordAdaptation(KEY root,QUALITY quality){
	return chordAdaptation[root][quality];
}
double ChordManager::getChordContinuity(KEY root,QUALITY quality){
	KEY prevRoot = prevChord.getRoot();
	QUALITY prevQuality = prevChord.getQuality();
	if(prevRoot==UNDEFINED_KEY){
		return 1;
	}
	return chordContinuity[prevRoot][prevQuality][root][quality];
}
double ChordManager::getChordLocation(KEY root,QUALITY quality,int measure){
	switch(measure%8){
	case 0:
		if(root==C && quality==MAJOR){
			return 1;
		}else if(root==E && quality==MINOR){
			return 0.6;
		}else if(root==A && quality==MINOR){
			return 0.8;
		}else if(root==F && quality==MAJOR){
			return 0.8;
		}else if(root==D && quality==MINOR){
			return 0.6;
		}
		break;
	case 7:
		if(root==C && quality==MAJOR){
			return 1;
		}
		break;
	}
	return 0;
}
int* ChordManager::getChordTones(QUALITY quality){
	return chordComposition[quality];
}
int* ChordManager::getAdditiveChordTones(ADDITIVE_QUALITY quality){
	return chordAddition[quality];
}
double ChordManager::evaluateChords(MusicChord *chords,int length){
	double value=1;
	value+=getChordAdaptation(chords[0].getRoot(),chords[0].getQuality());
	for(int i=1;i<length;i++){
		prevChord=chords[i-1];
		KEY root=chords[i].getRoot();
		QUALITY quality=chords[i].getQuality();
		value+=getChordAdaptation(root,quality)+getChordContinuity(root,quality)+getChordLocation(root,quality,i);;
	}
	return -1+value/(length*2+length/4);
}