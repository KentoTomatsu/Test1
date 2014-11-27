#pragma once
#include"common.h"
#include"ChordManager.h"
#include"MusicChord.h"
#include"Note.h"

enum TIME_SIGNATURE{
	NUMERATOR,DENOMINATOR,MIDI_CLOCK,ONE_BEAT
};
class Music
{
public:
	Music(void);
	~Music(void);
	void setScale(int scale);
	int getScale();
	void setTonic(KEY tonic);
	KEY getTonic();
	int getKeySignature();
	int getNoteLength();
	int getBasicNoteLength();
	void judgeTonality(); //曲のキーを推定する
	void shiftKey(int dif);
	void setNote(Note *note);
	Note *getHead();
	Note *getNote();
	void printNote();
	void printDifFirstNote();
	void printDifPrevNote();
	void printKey();
	void printTime();
	void printVelocity();
	void printDuration();
	int getTempo();
	void setTempo(int tempo);
	void selectTempo(Music *music,int length);
	int **encodeNote(int arrayLength);
	void decodeNote(int **notes,int length);
	int **generateRandomMusic(int length);
	void setChord(int **notes,int length);
	MusicChord *getChords();
	MusicChord getChord(int index);
	int getChordLength();
	void setTimeSignature(unsigned char *timeSignature);
	unsigned char *getTimeSignature();
	unsigned char getTimeSignature(TIME_SIGNATURE index);
private:
	int scale;
	KEY tonic;//曲のトニック
	Note *head;
	Note *note;
	MusicChord *chord;
	int noteLength;
	int basicNoteLength;
	int tempo;
	int chordLength;
	unsigned char *timeSignature;
	MyRandom random;

	void setNoteLength();
};