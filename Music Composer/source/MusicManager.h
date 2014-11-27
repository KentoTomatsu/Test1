#pragma once
#include<string.h>
#include"common.h"
#include"PropertiesManager.h"
#include"Music.h"
#include"MIDIData.h"
#include"Composer.h"
#include"FileManager.h"
#include"ChordManager.h"
#include"MusicChord.h"
#include"Note.h"

const int PAN=10;
const int REVERB=91;
const int CHORUS=93;
const int DEFAULT_VELOCITY=100;

class MusicManager
{
public:
	MusicManager(void);
	~MusicManager(void);
	void loadMusicFromFile(string *fileName,int length);	//MIDI�t�@�C���̃��[�h
	void composeMusic();	//�������
	void saveMusic();	//MIDI�t�@�C���ւ̕ۑ�
	void saveChord();	//�R�[�h�i�s���t�@�C���֕ۑ�
	void setSingleTone(Note *note,int,MIDITrack *track);	//MIDITrack�֒P���̒ǉ�
	void setChordTones(MusicChord *chord,int,MIDITrack *track);	//MIDITrack�փR�[�h�̒ǉ�
	void setMusic(Music *music,MIDITrack *track,int type);	//MIDITrack�֊y�Ȃ̒ǉ�
	Music extractNote(MIDIData *data);	//MIDI����f�[�^�̒��o
	void setPropertiesManager(PropertiesManager *propertiesManager);
private:
	Music *baseMusic;	//�x�[�X��
	Music *preMix;	//�����ő�
	Music *mixed;	//�ŏI��
	int musicLength;	//�x�[�X�Ȃ̐�
	int trackNum;
	int baseBeat;
	int baseNum;
	MIDIData* pMIDIData;	//MIDIData�|�C���^
	MyRandom random;
	PropertiesManager *propertiesManager;

	MIDITrack* getTrack(MIDITrack *track,int index);
	void setProperties(MIDITrack *track,Music *music);
	void setParameters(MIDITrack *track,int channel,int instrument,int pan,int chorus,int reverb);	//Instrument�̐ݒ�
	void deleteMIDIData();	//MIDIData�̃��������
};

