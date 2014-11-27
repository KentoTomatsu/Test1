#pragma once
#include"common.h"

class Note
{
public:
	Note(Note *note);
	Note(void);
	~Note(void);
	void setNote(int id,int key,int time,int vel,int duration);
	void generateNextNote();
	void setID(int id);
	void setKey(int key);
	void setVelocity(int vel);
	void setTime(int time);
	void setDuration(int duration);
	void setDifFirstNote(int dif);
	void setDifFirstMeasureNote(int dif);
	void setDifPrevNote(int dif);
	void setPrevNote(Note *note);
	void setNextNote(Note *note);
	int getID();
	int getKey();
	int getVelocity();
	int getTime();
	int getDuration();
	int getDifFirstNote();
	int getDifFirstMeasureNote();
	int getDifPrevNote();
	Note *getPrevNote();
	Note *getNextNote();
private:
	int id;
	int key;
	int vel;
	int time;
	int duration;
	int difFirstNote;
	int difFirstMeasureNote;
	int difPrevNote;
	Note *prevNote;
	Note *nextNote;
};