#include "MusicManager.h"
#include<stdlib.h>
#include"MyRandom.h"

MusicManager::MusicManager(void)
{
	pMIDIData = NULL;
	trackNum=12;
}
MusicManager::~MusicManager(void)
{
	delete[] baseMusic;
	deleteMIDIData();
}
void MusicManager::loadMusicFromFile(string *fileName,int length){
	musicLength = length;
	string directory = "resource/midi/";
	string extension = ".mid";
	baseMusic = new Music[musicLength];
	for(int i=0;i<musicLength;i++){
		pMIDIData = MIDIData_LoadFromSMF((directory+fileName[i]+extension).c_str());
		if (pMIDIData == NULL) {
			cout << "Fail in making MIDIData object." << endl;
			getchar();
			exit(1);
		}
		cout << "---song" << i << "---" << endl;
		MIDITrack *track = MIDIData_GetFirstTrack (pMIDIData);
		MIDIEvent *midiEvent = MIDITrack_GetFirstEvent(track);
		baseMusic[i] = extractNote(pMIDIData);
		baseMusic[i].judgeTonality();
		while(midiEvent!=NULL){
			if(midiEvent->m_lKind==MIDIEVENT_TIMESIGNATURE){
				baseMusic[i].setTimeSignature(midiEvent->m_pData);
			}
			midiEvent=midiEvent->m_pNextEvent;
		}
		deleteMIDIData();
	}
	propertiesManager->setBaseLength(musicLength);
	propertiesManager->setBaseMusicName(fileName);
}
void MusicManager::composeMusic(){
	Composer composer;
	composer.setPropertiesManager(propertiesManager);
	composer.mixByGA(baseMusic,musicLength);
	preMix = composer.getPreMix();
	preMix->setTempo(MIDIEVENT_MAXTEMPO/140);
	preMix->selectTempo(baseMusic,musicLength);
	mixed = composer.getMixed();
	mixed->setTempo(preMix->getTempo());
	//mixed->setTempo(MIDIEVENT_MAXTEMPO/100);
	//mixed->setTempo(MIDIEVENT_MAXTEMPO/140);
}
void MusicManager::saveMusic(){
	MIDITrack* pMIDITrack;
	pMIDIData = MIDIData_Create(MIDIDATA_FORMAT1, trackNum, MIDIDATA_TPQNBASE, 480);
	pMIDITrack = MIDIData_GetFirstTrack(pMIDIData);
	FileManager fileManager;
	setMusic(preMix,pMIDITrack,PRE_MIX);
	MIDIData_SaveAsSMF(pMIDIData, FileManager::getFileName(PRE_MIX).c_str()); 
	deleteMIDIData();
	pMIDIData = MIDIData_Create(MIDIDATA_FORMAT1, trackNum, MIDIDATA_TPQNBASE, 480);
	pMIDITrack = MIDIData_GetFirstTrack(pMIDIData);
	setMusic(mixed,pMIDITrack,MIXED);
	MIDIData_SaveAsSMF(pMIDIData, FileManager::getFileName(MIXED).c_str());
	deleteMIDIData();
	pMIDIData = MIDIData_Create(MIDIDATA_FORMAT1, trackNum, MIDIDATA_TPQNBASE, 480);
	pMIDITrack = MIDIData_GetFirstTrack(pMIDIData);
	setMusic(mixed,pMIDITrack,ARRANGED);
	MIDIData_SaveAsSMF(pMIDIData, FileManager::getFileName(ARRANGED).c_str());
}
void MusicManager::saveChord(){
	FileManager fileManager;
	fileManager.openStream(FileManager::getFileName(CHORD).c_str());
	fileManager.saveChord(mixed->getChords(),mixed->getChordLength());
}
void MusicManager::setChordTones(MusicChord *chord,int length,MIDITrack *track){
	for(int i=0;i<length;i++){
		cout << i << endl;
		for(int j=0;j<chord[i].getToneNum();j++){
			int key=chord[i].getNote(j).getKey();
			MIDITrack_InsertNote (track,480*i,0, key, 100, 480);		
		}
	}
}
void MusicManager::setMusic(Music *music,MIDITrack *track,int type){
	Note *note;
	int time=0;
	int chordTime=BASE_BEAT*BASE_NUM;
	int chordCount=0;

	note=music->getHead()->getNextNote()->getNextNote();
	setProperties(track,music);
	setParameters(getTrack(track,1),0,1,64,120,120);
	setParameters(getTrack(track,2),1,50,100,100,100);
	setParameters(getTrack(track,3),2,43,30,50,60);
	setParameters(getTrack(track,4),3,2,50,120,120);
	setParameters(getTrack(track,5),4,3,80,120,120);
	setParameters(getTrack(track,6),5,33,60,30,30);
	setParameters(getTrack(track,7),6,25,105,80,80);
	setParameters(getTrack(track,8),7,49,64,80,80);
	setParameters(getTrack(track,9),8,1,20,100,100);
	setParameters(getTrack(track,10),9,1,20,100,100);
	setParameters(getTrack(track,11),10,28,15,100,100);

	int *tones;
	MyRandom random;

	while(note!=NULL){
		MIDITrack_InsertNote(getTrack(track,1),time,0,note->getKey(),DEFAULT_VELOCITY+20,note->getDuration());
		if(type==ARRANGED){
			int rand = random.nextInt(5);
			switch(rand){
			case 0:
				rand=2;
				break;
			case 1:
				rand=4;
				break;
			case 2:
				rand=7;
				break;
			case 3:
				rand=9;
				break;
			default:
				rand=0;
			}
			//MIDITrack_InsertNote(getTrack(track,4),time,3,note->getKey()+rand,note->getVelocity()/4,note->getDuration());
			rand = random.nextInt(5);
			switch(rand){
			case 0:
				rand=-1;
				break;
			case 1:
				rand=-3;
				break;
			case 2:
				rand=-5;
				break;
			case 3:
				rand=2;
				break;
			default:
				rand=0;
			}
			//MIDITrack_InsertNote(getTrack(track,5),time,4,note->getKey()+rand,note->getVelocity()/4,note->getDuration());
		}
		time+=note->getDuration();
		if(type==ARRANGED){
			chordTime+=note->getDuration();
			if(chordTime>=BASE_BEAT*BASE_NUM){
				music->getChord(chordCount).printChord();
				tones = music->getChord(chordCount).getTones();
				for(int i=0;i<QUALITY_COMPOSITION_NUM;i++){
					if(tones[i]!=-1 && tones[i]<SCALE_LENGTH){
						MIDITrack_InsertNote(getTrack(track,2),BASE_BEAT*BASE_NUM*chordCount,1,tones[i]+SCALE_LENGTH*5,
							DEFAULT_VELOCITY/2,BASE_BEAT*BASE_NUM);
						MIDITrack_InsertNote(getTrack(track,3),BASE_BEAT*BASE_NUM*chordCount,2,tones[i]+SCALE_LENGTH*4,
							DEFAULT_VELOCITY*2/5,BASE_BEAT*BASE_NUM);
						MIDITrack_InsertNote(getTrack(track,8),BASE_BEAT*BASE_NUM*chordCount,7,tones[i]+SCALE_LENGTH*6,
							DEFAULT_VELOCITY*2/5,BASE_BEAT*BASE_NUM);
					}
				}
				/*for(int i=QUALITY_COMPOSITION_NUM;i<QUALITY_COMPOSITION_NUM+CHORD_ADDITION_LENGTH;i++){
					if(tones[i]!=-1 && tones[i]<SCALE_LENGTH){
						MIDITrack_InsertNote(getTrack(track,2),BASE_BEAT*BASE_NUM*chordCount,1,tones[i]+SCALE_LENGTH*5,
							DEFAULT_VELOCITY/2,BASE_BEAT*BASE_NUM);
					}
				}
				for(int i=0;i<BASE_NUM/8;i++){
					int base;
					do{
						base = tones[random.nextInt(QUALITY_COMPOSITION_NUM+CHORD_ADDITION_LENGTH)];
					}while(base==-1 && base>=SCALE_LENGTH);
					MIDITrack_InsertNote(getTrack(track,6),BASE_BEAT*BASE_NUM*chordCount+BASE_BEAT*8*i,5,base+SCALE_LENGTH*4,
						DEFAULT_VELOCITY/2,BASE_BEAT*BASE_NUM/2);
					MIDITrack_InsertNote(getTrack(track,9),BASE_BEAT*BASE_NUM*chordCount+BASE_BEAT*8*i,8,tones[i]+SCALE_LENGTH*5,
						DEFAULT_VELOCITY*3/5,BASE_BEAT*BASE_NUM/2);
				}
				for(int i=0;i<BASE_NUM/2;i++){
					int arpeggio;
					do{
						arpeggio = tones[random.nextInt(QUALITY_COMPOSITION_NUM+CHORD_ADDITION_LENGTH)];
					}while(arpeggio==-1 && arpeggio>=SCALE_LENGTH);
					MIDITrack_InsertNote(getTrack(track,7),BASE_BEAT*BASE_NUM*chordCount+BASE_BEAT*2*i,6,arpeggio+SCALE_LENGTH*5,
						DEFAULT_VELOCITY*2/5,BASE_BEAT*BASE_NUM/8);	
					do{
						arpeggio = tones[random.nextInt(QUALITY_COMPOSITION_NUM)];
					}while(arpeggio==-1 && arpeggio>=SCALE_LENGTH);
					MIDITrack_InsertNote(getTrack(track,9),BASE_BEAT*BASE_NUM*chordCount+BASE_BEAT*2*i,8,arpeggio+SCALE_LENGTH*6,
						DEFAULT_VELOCITY/2,BASE_BEAT*BASE_NUM/8);
					do{
						arpeggio = tones[random.nextInt(QUALITY_COMPOSITION_NUM)];
					}while(arpeggio==-1 && arpeggio>=SCALE_LENGTH);
					MIDITrack_InsertNote(getTrack(track,11),BASE_BEAT*BASE_NUM*chordCount+BASE_BEAT*i,10,arpeggio+SCALE_LENGTH*5,
						DEFAULT_VELOCITY/4,BASE_BEAT*BASE_NUM/16);
				}*/
				delete[] tones;
				chordCount++;
				chordTime=0;
			}
		}
		note=note->getNextNote();
	}
	if(type==ARRANGED){
		if(chordTime>0){
			music->getChord(chordCount).printChord();
			tones = music->getChord(chordCount).getTones();
			for(int i=0;i<QUALITY_COMPOSITION_NUM;i++){
				if(tones[i]!=-1 && tones[i]<SCALE_LENGTH){
					MIDITrack_InsertNote(getTrack(track,2),BASE_BEAT*BASE_NUM*chordCount,1,tones[i]+SCALE_LENGTH*5,
						DEFAULT_VELOCITY/2,BASE_BEAT*BASE_NUM);
					MIDITrack_InsertNote(getTrack(track,3),BASE_BEAT*BASE_NUM*chordCount,2,tones[i]+SCALE_LENGTH*4,
						DEFAULT_VELOCITY*2/5,BASE_BEAT*BASE_NUM);
					MIDITrack_InsertNote(getTrack(track,8),BASE_BEAT*BASE_NUM*chordCount,7,tones[i]+SCALE_LENGTH*6,
						DEFAULT_VELOCITY*2/5,BASE_BEAT*BASE_NUM);
				}
			}
			/*for(int i=QUALITY_COMPOSITION_NUM;i<QUALITY_COMPOSITION_NUM+CHORD_ADDITION_LENGTH;i++){
				if(tones[i]!=-1 && tones[i]<SCALE_LENGTH){
					MIDITrack_InsertNote(getTrack(track,2),BASE_BEAT*BASE_NUM*chordCount,1,tones[i]+SCALE_LENGTH*5,
						DEFAULT_VELOCITY/2,BASE_BEAT*BASE_NUM);
				}
			}
			for(int i=0;i<BASE_NUM/8;i++){
				int base;
				do{
					base = tones[random.nextInt(QUALITY_COMPOSITION_NUM+CHORD_ADDITION_LENGTH)];
				}while(base==-1 && base>=SCALE_LENGTH);
				MIDITrack_InsertNote(getTrack(track,6),BASE_BEAT*BASE_NUM*chordCount+BASE_BEAT*8*i,5,base+SCALE_LENGTH*4,
					DEFAULT_VELOCITY*3/5,BASE_BEAT*BASE_NUM/2);	
				MIDITrack_InsertNote(getTrack(track,9),BASE_BEAT*BASE_NUM*chordCount+BASE_BEAT*8*i,8,tones[i]+SCALE_LENGTH*5,
						DEFAULT_VELOCITY*3/5,BASE_BEAT*BASE_NUM/2);
			}
			for(int i=0;i<BASE_NUM/2;i++){
				int arpeggio;
				do{
					arpeggio = tones[random.nextInt(QUALITY_COMPOSITION_NUM+CHORD_ADDITION_LENGTH)];
				}while(arpeggio==-1 && arpeggio>=SCALE_LENGTH);
				MIDITrack_InsertNote(getTrack(track,7),BASE_BEAT*BASE_NUM*chordCount+BASE_BEAT*2*i,6,arpeggio+SCALE_LENGTH*5,
					DEFAULT_VELOCITY*2/5,BASE_BEAT*BASE_NUM/8);
				do{
					arpeggio = tones[random.nextInt(QUALITY_COMPOSITION_NUM)];
				}while(arpeggio==-1 && arpeggio>=SCALE_LENGTH);
				MIDITrack_InsertNote(getTrack(track,9),BASE_BEAT*BASE_NUM*chordCount+BASE_BEAT*2*i,8,arpeggio+SCALE_LENGTH*6,
					DEFAULT_VELOCITY/2,BASE_BEAT*BASE_NUM/8);
				do{
					arpeggio = tones[random.nextInt(QUALITY_COMPOSITION_NUM)];
				}while(arpeggio==-1 && arpeggio>=SCALE_LENGTH);
				MIDITrack_InsertNote(getTrack(track,11),BASE_BEAT*BASE_NUM*chordCount+BASE_BEAT*i,10,arpeggio+SCALE_LENGTH*5,
					DEFAULT_VELOCITY/4,BASE_BEAT*BASE_NUM/16);
			}*/
			delete[] tones;
		}
	}
	int length=BASE_BEAT*GENE_SIZE;
	MIDITrack_InsertEndofTrack(track,length);
	MIDITrack_InsertEndofTrack(getTrack(track,1),length);
	MIDITrack_InsertEndofTrack(getTrack(track,2),length);
	MIDITrack_InsertEndofTrack(getTrack(track,3),length);
	MIDITrack_InsertEndofTrack(getTrack(track,4),length);
	MIDITrack_InsertEndofTrack(getTrack(track,5),length);
	MIDITrack_InsertEndofTrack(getTrack(track,6),length);
	MIDITrack_InsertEndofTrack(getTrack(track,7),length);
	MIDITrack_InsertEndofTrack(getTrack(track,8),length);
	MIDITrack_InsertEndofTrack(getTrack(track,9),length);
	MIDITrack_InsertEndofTrack(getTrack(track,10),length);
	MIDITrack_InsertEndofTrack(getTrack(track,11),length);
}
Music MusicManager::extractNote(MIDIData *data){
	MIDITrack* pTrack = data->m_pFirstTrack;
	Music music;
	Note *head= new Note(NULL);
	Note *p = new Note(head);
	p=head;
	int i=0;
	int firstKey=0;
	int prevKey=0;
	int startTime=0;
	int baseBeat=MIDIData_GetTimeResolution(data)/4; //16•ª‰¹•„‚Ìtick‚ðŽæ“¾‚µ‚Ä‚¢‚éH

	int minTime=1000000;
	int prevTime=0;

	/*while(pTrack!=NULL){
		MIDIEvent* pEvent = pTrack->m_pFirstEvent;
		do{
			if(MIDIEvent_IsNoteOn(pEvent) || MIDIEvent_IsNoteOff(pEvent)){
				int time=MIDIEvent_GetTime(pEvent);	
				if(MIDIEvent_IsNoteOn(pEvent)){
					if(i==0){
						startTime=time;
					}
					prevTime=time;
					i++;
				}else{
					int dif=time-prevTime;
					if(dif!=0 && dif<minTime){
						minTime=dif;
						cout << "min duration: " << dif << endl;
					}
				}				
			}
			pEvent=pEvent->m_pNextEvent;
		}while(pEvent->m_pNextEvent!=NULL);
		pTrack=pTrack->m_pNextTrack;
	}
	pTrack = data->m_pFirstTrack;*/
	while(pTrack!=NULL){
		MIDIEvent* pEvent = pTrack->m_pFirstEvent;
		do{
			if(MIDIEvent_IsNoteOn(pEvent) || MIDIEvent_IsNoteOff(pEvent)){
				int time=MIDIEvent_GetTime(pEvent);	
				int vel=MIDIEvent_GetVelocity(pEvent);
				int key=MIDIEvent_GetKey(pEvent);
				int duration=MIDIEvent_GetDuration(pEvent);
				if(MIDIEvent_IsNoteOn(pEvent)){
					Note *note = new Note(p);
					duration=BASE_BEAT*4;
					if(i==0){
						firstKey=key;
						prevKey=key;
						startTime=time;
					}
					note->setNote(i,key,time-startTime,vel,duration);
					note->setDifFirstNote(key-firstKey);
					note->setDifPrevNote(key-prevKey);
					prevKey=key;
					p->setNextNote(note);
					p=note;
					i++;
				}else{
					if(p!=head){
						int dif=p->getTime()-p->getPrevNote()->getTime();
						if(dif%baseBeat<baseBeat/2){
							dif=dif/baseBeat;
						}else{
							dif=dif/baseBeat+1;
						}
						p->getPrevNote()->setDuration(max(BASE_BEAT,dif*BASE_BEAT));
					}				
				}
			}else if(MIDIEvent_IsTempo(pEvent)){
				music.setTempo(MIDIEvent_GetTempo(pEvent));
			}
			pEvent=pEvent->m_pNextEvent;
		}while(pEvent->m_pNextEvent!=NULL);
		pTrack=pTrack->m_pNextTrack;
	}
	p->setNextNote(NULL);
	music.setNote(head);
	return music;
}
void MusicManager::setSingleTone(Note *note,int length,MIDITrack *track){
	for(int i=0;i<length;i++){
		int key=note[i].getKey();
		MIDITrack_InsertNote (track, 0, 0, key, 100, 360);
	}
}
MIDITrack* MusicManager::getTrack(MIDITrack *track,int index){
	MIDITrack *p = track;
	for(int i=0;i<index && p!=NULL;i++){
		p=p->m_pNextTrack;
	}
	return p;
}
void MusicManager::setProperties(MIDITrack *track,Music *music){
	MIDITrack_InsertTempo (track, 0, music->getTempo());
	MIDITrack_InsertKeySignature(track,0,music->getKeySignature(),music->getScale());
}
void MusicManager::setParameters(MIDITrack *track,int channel,int instrument,int pan,int chorus,int reverb){
	MIDITrack_InsertProgramChange (track, 0, channel, instrument);
	MIDITrack_InsertControlChange(track,0,channel,PAN,pan);
	MIDITrack_InsertControlChange(track,0,channel,CHORUS,chorus);
	MIDITrack_InsertControlChange(track,0,channel,REVERB,reverb);
}
void MusicManager::deleteMIDIData(){
	if(pMIDIData!=NULL){
		MIDIData_Delete (pMIDIData);
		pMIDIData = NULL;
	}
}
void MusicManager::setPropertiesManager(PropertiesManager *manager){
	propertiesManager=manager;
}