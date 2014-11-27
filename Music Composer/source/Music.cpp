#include "Music.h"

Music::Music(void)
{
}
Music::~Music(void)
{
}
void Music::setScale(int scale){
	this->scale=scale;
}
int Music::getScale(){
	return scale;
}
void Music::setTonic(KEY tonic){
	this->tonic=tonic;
}
KEY Music::getTonic(){
	return tonic;
}
int Music::getKeySignature(){
	if(scale==MAJOR){
		switch(tonic){
		case C:
			return 0;
		case Cs:
			return -5;
		case D:
			return 2;
		case Ds:
			return -3;
		case E:
			return 4;
		case F:
			return -1;
		case Fs:
			return 6;
		case G:
			return 1;
		case Gs:
			return -4;
		case A:
			return 3;
		case As:
			return -2;
		case B:
			return 5;
		}
	}else{
		switch(tonic){
		case C:
			return -3;
		case Cs:
			return 4;
		case D:
			return -1;
		case Ds:
			return 6;
		case E:
			return 1;
		case F:
			return -4;
		case Fs:
			return 3;
		case G:
			return -2;
		case Gs:
			return 5;
		case A:
			return 0;
		case As:
			return -5;
		case B:
			return 2;
		}
	}
	return 0;
}
int Music::getNoteLength(){
	return noteLength;
}
int Music::getBasicNoteLength(){
	return basicNoteLength;
}
void Music::judgeTonality(){
	int keyFreq[SCALE_LENGTH];
	int firstKey=UNDEFINED_KEY,lastKey;
	for(int i=0;i<SCALE_LENGTH;i++){
		keyFreq[i]=0;
	}
	Note *p=head->getNextNote();
	while(p!=NULL){
		if(p->getKey()!=-1){
			keyFreq[p->getKey()%SCALE_LENGTH]++;
			if(firstKey==UNDEFINED_KEY){
				firstKey=p->getKey()%SCALE_LENGTH;
			}
		}
		lastKey=p->getKey()%SCALE_LENGTH;
		p=p->getNextNote();
	}
	for(int i=0;i<SCALE_LENGTH;i++){
		cout << keyName[i] << ":" <<  keyFreq[i] << " ";
	}
	cout << endl;
	int pattern[SCALE_LENGTH] = {1,0,1,0,1,1,0,1,0,1,0,1};
	int start=0,max=-1;
	for(int i=0;i<SCALE_LENGTH;i++){
		int index=i,fitness=0;
		for(int j=0;j<SCALE_LENGTH;j++){
			fitness+=keyFreq[index]*pattern[j];
			index++;
			if(index>=SCALE_LENGTH){
				index=0;
			}
		}
		if(fitness>max){
			max=fitness;
			start=i;		
		}
	}
	if(firstKey==start){
		tonic=(KEY)start;
		scale=MAJOR;
	}else if(firstKey==start+9){
		tonic=(KEY)((start+9)%SCALE_LENGTH);
		scale=MINOR;
	}else if(lastKey==start){
		tonic=(KEY)start;
		scale=MAJOR;
	}else if(lastKey==start+9){
		tonic=(KEY)((start+9)%SCALE_LENGTH);
		scale=MINOR;
	}else{
		tonic=(KEY)start;
		scale=MAJOR;
	}
	cout << "Scale: ";
	if(scale==MAJOR){
		cout <<  keyName[tonic] << " Major" << endl;
	}else{
		cout <<  keyName[tonic] << " minor" << endl;
	}	
}

void Music::setNote(Note *note){
	head = new Note(NULL);
	head->setNextNote(note);
	this->note=note;
	setNoteLength();
}
Note* Music::getHead(){
	return head;
}
Note* Music::getNote(){
	return note;
}
int Music::getTempo(){
	return tempo;
}
void Music::setTempo(int t){
	tempo=t;
}
void Music::shiftKey(int dif){
	Note *p=head->getNextNote()->getNextNote();
	//cout << "ShiftKey\n";
	while(p!=NULL){
		p->setKey(p->getKey()+dif);
		p=p->getNextNote();
	}
	//cout << endl;
}
void Music::printNote(){
	Note *p=head->getNextNote()->getNextNote();
	int i=0;
	cout << "---Note---" << endl;
	while(p!=NULL){
		cout << "ID: " << p->getID() << " key: " << p->getKey() << ": " << keyName[(p->getKey()%SCALE_LENGTH)] 
			<< " time: " << p->getTime() << " vel: " << p->getVelocity() << " dur: " << p->getDuration() << endl;
		p=p->getNextNote();
	}
	cout << endl;
}
void Music::printDifFirstNote(){
	Note *p=head->getNextNote()->getNextNote();
	int i=0;
	cout << "---Difference btwn FocusedNote n FirstNote---" << endl;
	while(p!=NULL){
		cout << "ID: " << p->getID() << " key: " << p->getDifFirstNote() << " ";
		p=p->getNextNote();
		i++;
		if(i%5==0){
			cout << endl;
		}
	}
	cout << endl;
}
void Music::printDifPrevNote(){
	Note *p=head->getNextNote()->getNextNote();
	int i=0;
	cout << "---Difference btwn FocusedNote n PrevNote---" << endl;
	while(p!=NULL){
		cout << "ID: " << p->getID() << " key: " << p->getDifPrevNote() << " ";
		p=p->getNextNote();
		i++;
		if(i%5==0){
			cout << endl;
		}
	}
	cout << endl;
}
void Music::printKey(){
	Note *p=head->getNextNote()->getNextNote();
	int i=0;
	cout << "---Note---" << endl;
	while(p!=NULL){
		cout << "ID: " << p->getID() << " key: " << p->getKey() << ": " << keyName[(p->getKey()%SCALE_LENGTH)] << endl;
		p=p->getNextNote();
		i++;
		if(i%5==0){
			cout << endl;
		}
	}
	cout << endl;
}
void Music::printTime(){
	Note *p=head->getNextNote()->getNextNote();
	int i=0;
	cout << "---Time---" << endl;
	while(p!=NULL){
		cout << "ID: " << p->getID() << " time: " << p->getTime() << " ";
		p=p->getNextNote();
		i++;
		if(i%3==0){
			cout << endl;
		}
	}
	cout << endl;
}
void Music::printVelocity(){
	Note *p=head->getNextNote()->getNextNote();
	int i=0;
	cout << "---Velocity---" << endl;
	while(p!=NULL){
		cout << "ID: " << p->getID() << " vel: " << p->getVelocity() << " ";
		p=p->getNextNote();
		i++;
		if(i%5==0){
			cout << endl;
		}
	}
	cout << endl;
}
void Music::printDuration(){
	Note *p=head->getNextNote()->getNextNote();
	int i=0;
	cout << "---Duration---" << endl;
	while(p!=NULL){
		cout << "ID: " << p->getID() << " dur: " << p->getDuration() << " ";
		p=p->getNextNote();
		i++;
		if(i%5==0){
			cout << endl;
		}
	}
	cout << endl;
}
int** Music::encodeNote(int arrayLength){
	int** notes = new int*[arrayLength];
	int i;
	for(i=0;i<arrayLength;i++){
		notes[i] = new int[NOTE_CONTENTS_LENGTH];
	}
	i=0;
	Note *p=head->getNextNote()->getNextNote();
	while(p!=NULL){
		int riseCount=p->getDuration()/BASE_BEAT;
		for(int j=0;j<riseCount;j++,i++){
			notes[i][NOTE_KEY]=p->getKey();
			notes[i][NOTE_RISING]=j;
		}
		p=p->getNextNote();
	}
	while(i<arrayLength){
		for(int j=0;j<NOTE_CONTENTS_LENGTH;j++){
			notes[i][j]=-1;
		}
		i++;
	}
	return notes;
}
void Music::decodeNote(int **notes,int length){
	Note *h= new Note(NULL);
	Note *p = new Note(h);
	p=h;
	int firstKey,prevKey;
	int index=0,i=0;
	int abTime=0;
	while(i<length){
		Note *note = new Note(p);
		int key=notes[i][NOTE_KEY];
		int vel=100;
		if(i==0){
			firstKey=key;
			prevKey=key;
		}
		int duration=0;
		do{
			duration+=BASE_BEAT;
			i++;
		}while(i<length && notes[i][NOTE_RISING]!=0);
		note->setNote(index,key,abTime,vel,duration);
		note->setDifFirstNote(key-firstKey);
		note->setDifPrevNote(key-prevKey);
		abTime+=duration;
		prevKey=key;
		p->setNextNote(note);
		p=note;
		index++;
	}
	p->setNextNote(NULL);
	setChord(notes,length);
	setNote(h);
}
MusicChord* Music::getChords(){
	return chord;
}
MusicChord Music::getChord(int index){
	return chord[index];
}
void Music::setChord(int **notes,int length){
	ChordManager m;
	m.setNotes(notes);
	m.setMusicLength(length);
	m.setScale(scale);
	chord= m.judgeChord();
	chordLength = length/BASE_NUM;
}
void Music::setNoteLength(){
	Note *p=head->getNextNote()->getNextNote();
	int i=0,time=0;
	while(p!=NULL){
		time+=p->getDuration();
		p=p->getNextNote();
		i++;
	}
	noteLength=i;
	basicNoteLength=time/BASE_BEAT;
}
void Music::selectTempo(Music *music,int length){
	int rand=random.nextInt(length+1);
	if(rand<length){
		setTempo(music[rand].getTempo());
	}else{
		int tmp=0;
		for(int i=0;i<length;i++){
			tmp+=music[i].getTempo();
		}
		setTempo(tmp/length);
	}
}
int** Music::generateRandomMusic(int length){
	int** notes = new int*[length];
	for(int i=0;i<length;i++){
		notes[i] = new int[NOTE_CONTENTS_LENGTH];
	}
	int key=random.nextInt(SCALE_LENGTH)+60;
	tonic = (KEY)(key%SCALE_LENGTH);
	scale=MAJOR;
	int len=0;
	for(int i=0;i<length;i++){
		if(random.nextInt(2)){
			key+=random.nextInt(SCALE_LENGTH);
		}else{
			key-=random.nextInt(SCALE_LENGTH);
		}
		if(key<0 || key>127){
			key=random.nextInt(SCALE_LENGTH)+60;
		}
		int duration = pow(2.0,random.nextInt(5));
		int count=0;
		while(i<length && count<duration){
			notes[i][NOTE_KEY]=key;
			notes[i][NOTE_RISING]=count;
			count++;
			i++;
		}
		len++;
		i--;
	
	}
	noteLength = len;
	basicNoteLength=length;
	return notes;
}
int Music::getChordLength(){
	return chordLength;
}
void Music::setTimeSignature(unsigned char *timeSignature){
	this->timeSignature=timeSignature;
}
unsigned char *Music::getTimeSignature(){
	return timeSignature;
}
unsigned char Music::getTimeSignature(TIME_SIGNATURE index){
	return timeSignature[index];
}