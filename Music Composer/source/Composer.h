#pragma once
#include"common.h"
#include"Music.h"
#include"PropertiesManager.h"
#include"GAForMusic.h"
class Composer
{
public:
	Composer(void);
	~Composer(void);
	void modulateMusic(Music *music,int length);	//music[0]のスケールに合わせる
	void mixByGA(Music *music,int length);	//GAによる合成
	Music *getPreMix();	//初期個体の獲得
	Music *getMixed();	//最終個体の獲得
	void setPropertiesManager(PropertiesManager *propertiesManager);
private:
	Music preMix;	//初期個体
	Music mixed;	//最終個体
	GAForMusic ga;
	PropertiesManager *propertiesManager;
	KEY tonic;
	int scale;

	void setProperties();
};

