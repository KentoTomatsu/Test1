#pragma once
#include "common.h"
#include "MusicChord.h"

using namespace std;

const double MIN_CHORD_ADAPTATION = 0.02;
const double THRESHOLD_OF_ADAPTATION = 0.3;
const double MIN_CHORD_CONTINUITY = 0.02;
const double THRESHOLD_OF_CONTINUITY = 0.4;

class ChordManager
{
public:
	ChordManager(void);
	~ChordManager(void);
	void setChord(MusicChord *chord);
	void setQuality();
	int getQualityToneNum(QUALITY);
	void printChord(MusicChord *chord,int);
	MusicChord* judgeChord();
	MusicChord judgeSingleChord(int *tones);
	MusicChord getMatchChord(int *noteCount);
	bool isIncludingTone(int root,int quality,int key);
	bool isIncludingAdditiveTone(int root,int quality,int key);
	double getChordAdaptation(KEY root,QUALITY quality);
	double getChordContinuity(KEY root,QUALITY quality);
	double getChordLocation(KEY root,QUALITY quality,int measure);
	void setNotes(int **notes);
	void setMusicLength(int length);
	void setScale(int scale);
	int* getChordTones(QUALITY quality);
	int* getAdditiveChordTones(ADDITIVE_QUALITY additiveQuality);
	int* getIntervals(MusicChord chord,int key);
	double evaluateChords(MusicChord *chords,int length);
private:
	int **notes;
	int musicLength;
	int scale;
	//KEY tonic;
	//MusicChord *chord;
	MusicChord prevChord;
	MyRandom random;
	int chordComposition[QUALITY_LENGTH][QUALITY_COMPOSITION_NUM];
	int chordAddition[ADDITIVE_QUALITY_LENGTH][CHORD_ADDITION_LENGTH];
	double chordAdaptation[SCALE_LENGTH][QUALITY_LENGTH];
	double chordContinuity[SCALE_LENGTH][QUALITY_LENGTH][SCALE_LENGTH][QUALITY_LENGTH];
	double chordFitness[SCALE_LENGTH][QUALITY_LENGTH];

	void setChordCompositon();
	void setChordAdaptation();
	void setChordContinuity();
	ADDITIVE_QUALITY setAdditiveQuality(KEY root,QUALITY quality,int *tones);
};