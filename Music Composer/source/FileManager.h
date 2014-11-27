#pragma once
#include"MIDIData.h"
#include"MusicChord.h"
#include"common.h"
#include<string>
#include <fstream>

enum FILETYPE{
	PRE_MIX,MIXED,ARRANGED,FITNESS_FILE,MUSIC_THEORY_FILE,GLOBAL_FILE,LOCAL_FILE,CHORD,PROPERTIES
};

class FileManager
{
public:
	FileManager(void);
	~FileManager(void);
	static string getFileName(FILETYPE fileType);
	void saveCSV(double *value,int length);	//csvÉfÅ[É^Çsave
	void saveChord(MusicChord *chord,int length);
	void saveProperties(string data);
	void openStream(const char *file);
private:
	ofstream ofs;
};

