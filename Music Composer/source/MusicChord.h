#pragma once
#include"common.h"
#include"Note.h"

class MusicChord
{
public:
	MusicChord(KEY root=UNDEFINED_KEY,QUALITY quality=UNDEFINED_QUALITY,ADDITIVE_QUALITY additiveQuality=SIMPLE,int octave=4);
	~MusicChord(void);
	void setChord(KEY root,QUALITY quality,int octave);
	void setAdditiveQuality(ADDITIVE_QUALITY additiveQuality);
	int *getTones();
	void setToneNum(int toneNum);
	KEY getRoot();
	QUALITY getQuality();
	ADDITIVE_QUALITY getAdditiveQuality();
	int getToneNum();
	void printChord();
	string getChordName();
	void setNote(Note *note);
	Note *getNote();
	Note getNote(int index);
private:
	KEY root;
	QUALITY quality;
	ADDITIVE_QUALITY additiveQuality;
	int octave;
	int toneNum;
	Note *note;
};