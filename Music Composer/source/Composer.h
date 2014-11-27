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
	void modulateMusic(Music *music,int length);	//music[0]�̃X�P�[���ɍ��킹��
	void mixByGA(Music *music,int length);	//GA�ɂ�鍇��
	Music *getPreMix();	//�����̂̊l��
	Music *getMixed();	//�ŏI�̂̊l��
	void setPropertiesManager(PropertiesManager *propertiesManager);
private:
	Music preMix;	//������
	Music mixed;	//�ŏI��
	GAForMusic ga;
	PropertiesManager *propertiesManager;
	KEY tonic;
	int scale;

	void setProperties();
};

