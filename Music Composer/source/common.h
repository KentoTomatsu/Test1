#pragma once
#include<iostream>
#include "MyRandom.h"
#define _USE_MATH_DEFINES
using namespace std;

const int LOOP=1;	//�v���O�������s��
const int PAIR=10;
const int SCALE_LENGTH=12;	//1�I�N�^�[�u�̉���
const int QUALITY_LENGTH=2;	//MAJ, min
const int ADDITIVE_QUALITY_LENGTH=8;	//�t�������ւ��R�[�h�̎��
const int QUALITY_COMPOSITION_NUM=3;	//MAJ, min�̍\�����̐�
const int CHORD_ADDITION_LENGTH=2;	//�t�����̍\�����̐�
const int BASE_BEAT=120;	//��{����(16��)
const int BASE_NUM=16;	//1��������{�����ō\�������Ƃ��̉�����
const int METER=BASE_BEAT*BASE_NUM;	//���q
const int METER_UNIT=4;
const int KEY_NUM=128;	//�����̐�
const int MEASURE_UNIT=8;
const int COLOR_NOTES=7;
const double INVERSE_MEASURE_UNIT=1.0/MEASURE_UNIT;
const int MEASURE_NUM=8;
const int GENE_SIZE=BASE_NUM*MEASURE_NUM;	//��`�q��
const int MAJOR=0;
const int MINOR=1;
enum KEY{
	C,Cs,D,Ds,E,F,Fs,G,Gs,A,As,B,UNDEFINED_KEY=-1
};
enum QUALITY{
	MAJ,MIN,UNDEFINED_QUALITY=-1
};
enum ADDITIVE_QUALITY{
	SIMPLE,ADDED_NINETH,SIXTH,SIXTH_ADD_NINE,SEVENTH,MAJOR_SEVENTH,SEVENTH_NINETH,MAJOR_SEVENTH_NINETH
};
const char keyName[SCALE_LENGTH][3]={"C","C#","D","D#","E","F","F#","G","G#","A","A#","B"};
const char qualityName[QUALITY_LENGTH][4]={"","m"};
const char additiveQualityName[ADDITIVE_QUALITY_LENGTH][6]={"","add9","6","69","7","M7","7(9)","M7(9)"};
enum NOTE_CONTENTS{
	NOTE_KEY,NOTE_RISING,NOTE_CONTENTS_LENGTH
};
enum SUPERORDINATE_FITNESS{
	MUSIC_THEORY,GLOBAL,LOCAL,SUPERORDINATE_FITNESS_LENGTH
};
enum GLOBAL_SIMILARITY{
	CHORD_SIMILARITY,GLOBAL_SIMILARITY_LENGTH
};
enum FITNESS{
	HARMONY,SMOOTH,SD,RHYTHMIC_CONTINUITY,CHORD_ADAPTATION,IMPLUSE,FITNESS_LENGTH
};
const char FITNESS_NAME[FITNESS_LENGTH][32]={
	"Harmony","Smooth Melody (Pitch)","Pitch Range","Smooth Melody (Duration)","Chord Adaptation","Rhythmic Similarity"
};
const char GLOBAL_SIMILARITY_NAME[GLOBAL_SIMILARITY_LENGTH][32]={
	"Chord"
};