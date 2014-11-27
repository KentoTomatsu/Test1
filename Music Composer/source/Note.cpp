#include "Note.h"

Note::Note(Note *prev)
{
	prevNote = prev;
	id=-1;
	key=UNDEFINED_KEY;
	duration=0;
	vel=0;
	time=0;
	difFirstNote=0;
	difPrevNote=0;
}
Note::Note(void)
{
}
Note::~Note(void)
{
}
void Note::generateNextNote(){
	nextNote = new Note(this);
}
void Note::setNote(int i,int k,int t,int v,int d){
	id=i;
	key=k;
	time=t;
	vel=v;
	duration=d;
}
void Note::setID(int id){
	this->id=id;
}
void Note::setKey(int key){
	this->key=key;
}
void Note::setVelocity(int vel){
	this->vel=vel;
}
void Note::setTime(int time){
	this->time=time;
}
void Note::setDuration(int duration){
	this->duration=duration;
}
void Note::setDifFirstNote(int dif){
	this->difFirstNote=dif;
}
void Note::setDifFirstMeasureNote(int dif){
	this->difFirstMeasureNote=dif;
}
void Note::setDifPrevNote(int dif){
	this->difPrevNote=dif;
}
void Note::setPrevNote(Note *note){
	prevNote=note;
}
void Note::setNextNote(Note *note){
	nextNote=note;
}
int Note::getID(){
	return id;
}
int Note::getKey(){
	return key;
}
int Note::getVelocity(){
	return vel;
}
int Note::getTime(){
	return time;
}
int Note::getDuration(){
	return duration;
}
int Note::getDifFirstNote(){
	return difFirstNote;
}
int Note::getDifFirstMeasureNote(){
	return difFirstMeasureNote;
}
int Note::getDifPrevNote(){
	return difPrevNote;
}
Note *Note::getPrevNote(){
	return prevNote;
}
Note *Note::getNextNote(){
	return nextNote;
}