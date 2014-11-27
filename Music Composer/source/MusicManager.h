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
	void loadMusicFromFile(string *fileName,int length);	//MIDIファイルのロード
	void composeMusic();	//自動作曲
	void saveMusic();	//MIDIファイルへの保存
	void saveChord();	//コード進行をファイルへ保存
	void setSingleTone(Note *note,int,MIDITrack *track);	//MIDITrackへ単音の追加
	void setChordTones(MusicChord *chord,int,MIDITrack *track);	//MIDITrackへコードの追加
	void setMusic(Music *music,MIDITrack *track,int type);	//MIDITrackへ楽曲の追加
	Music extractNote(MIDIData *data);	//MIDIからデータの抽出
	void setPropertiesManager(PropertiesManager *propertiesManager);
private:
	Music *baseMusic;	//ベース曲
	Music *preMix;	//初期固体
	Music *mixed;	//最終個体
	int musicLength;	//ベース曲の数
	int trackNum;
	int baseBeat;
	int baseNum;
	MIDIData* pMIDIData;	//MIDIDataポインタ
	MyRandom random;
	PropertiesManager *propertiesManager;

	MIDITrack* getTrack(MIDITrack *track,int index);
	void setProperties(MIDITrack *track,Music *music);
	void setParameters(MIDITrack *track,int channel,int instrument,int pan,int chorus,int reverb);	//Instrumentの設定
	void deleteMIDIData();	//MIDIDataのメモリ解放
};

