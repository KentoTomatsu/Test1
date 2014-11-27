#include "FileManager.h"
#include <sstream>

FileManager::FileManager(void)
{
}
FileManager::~FileManager(void)
{
	if(ofs.is_open()){
		ofs.close();
	}
}
void FileManager::openStream(const char *file){
	ofs.open(file,ios::app);
}
string FileManager::getFileName(FILETYPE fileType){
	string fileHead,fileName,extension;
	switch(fileType){
	case PRE_MIX:
		fileHead="product/midi/preMix";
		extension=".mid";
		break;
	case MIXED:
		fileHead="product/midi/mixed";
		extension=".mid";
		break;
	case ARRANGED:
		fileHead="product/midi/arranged";
		extension=".mid";
		break;
	case FITNESS_FILE:
		fileHead="product/fitness/fitness";
		extension=".csv";
		break;
	case MUSIC_THEORY_FILE:
		fileHead="product/fitness/music_theory/music_theory_fitness";
		extension=".csv";
		break;
	case GLOBAL_FILE:
		fileHead="product/fitness/global_similarity/global_sim_fitness";
		extension=".csv";
		break;
	case LOCAL_FILE:
		fileHead="product/fitness/local_similarity/local_sim_fitness";
		extension=".csv";
		break;
	case CHORD:
		fileHead="product/chord/chords";
		extension=".csv";
		break;
	case PROPERTIES:
		fileHead="product/properties/properties";
		extension=".csv";
		break;
	default:
		fileHead="product/error";
		extension=".dat";
	}
	int i=0;
	ostringstream value;
	while(true){
		value << i;
		ifstream ifs(fileHead+value.str()+extension);
		if(ifs==NULL){
			fileName=fileHead+value.str()+extension;
			break;
		}
		value.str("");
		i++;
	}
	cout << fileName.c_str() << endl;
	return fileName;
}
void FileManager::saveCSV(double *value,int length){
	for(int i=0;i<length;i++){
		ofs << value[i] << ",";
	}
	ofs << endl;
}
void FileManager::saveChord(MusicChord *chord,int length){
	for(int i=0;i<length;i++){
		ofs << chord[i].getChordName() << "," << endl;
	}
}
void FileManager::saveProperties(string data){
	ofs << data << endl;
}