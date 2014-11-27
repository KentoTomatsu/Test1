#include<string>
#include<Shlwapi.h>
#include<Windows.h>
#include"common.h"
#include"PropertiesManager.h"
#include"Measure.h"
#include"MusicManager.h"


int main(){
	PropertiesManager properties;
	Measure measure;
	int musicLength[PAIR]={1,2,1,1,1,2,2,2,2,2};
	string fileName[PAIR][4];
	//fileName[0][1]="jingle_bells";
	//fileName[0][1]="Deck_the_Halls";
	//fileName[0][0]="jingle_bells";
	//fileName[0][1]="Deck_the_Halls";
	//fileName[1][0]="jingle_bells";
	//fileName[0][0]="entertainer_a_low";
	//fileName[1][0]="entertainer_chorus_low";
	//fileName[0][0]="jingle_bells_chorus";
	//fileName[0][1]="Deck_the_Halls";
	fileName[0][0]="entertainer_chorus_low";
	fileName[1][0]="jingle_bells_chorus";
	fileName[1][1]="Deck_the_Halls";
	fileName[2][0]="jingle_bells_chorus";
	fileName[3][0]="Deck_the_Halls";
	fileName[4][0]="little_hans";
	fileName[5][0]="entertainer_chorus_low";
	fileName[6][0]="entertainer_chorus_low";
	fileName[7][0]="entertainer_chorus_low";
	fileName[5][1]="jingle_bells_chorus";
	fileName[6][1]="Deck_the_Halls";
	fileName[7][1]="little_hans";
	fileName[8][0]="jingle_bells_chorus";
	fileName[9][0]="Deck_the_Halls";
	fileName[8][1]="little_hans";
	fileName[9][1]="little_hans";
	/*fileName[1][0]="entertainer_chorus_low";
	fileName[1][1]="little_hans";
	fileName[2][0]="jingle_bells_chorus";
	fileName[2][1]="little_hans";
	fileName[3][0]="Deck_the_Halls";
	fileName[3][1]="little_hans";*/
	/*fileName[0][0]="jingle_bells_chorus";
	fileName[1][0]="Deck_the_Halls";
	fileName[0][0]="tulip";
	fileName[1][0]="entertainer_chorus_low";
	fileName[1][1]="tulip";
	fileName[0][0]="jingle_bells_chorus";
	fileName[0][1]="tulip";
	fileName[1][0]="Deck_the_Halls";
	fileName[1][1]="tulip";
	*/
	//fileName[3][1]="Deck_the_Halls";
	//fileName[0][0]="jingle_bells";
	
	//fileName[1][1]="yesterday_once_more";
	/*fileName[2][1]="Deck_the_Halls";
	fileName[3][1]="jingle_bells";
	fileName[3][0]="canon";
	fileName[4][1]="yesterday_once_more";
	fileName[4][0] = "summer";*/
	
	//fileName[0][0] = "tulip";
	//fileName[2][1] = "tulip";
	//fileName[1][0] = "tulip";
	//fileName[1][0] = "summer";
	//fileName[4][0]="canon";
	//fileName[4][1]="summer";
	for(int i=0;i<PAIR;i++){
		cout << "---PAIR" << i << "---" << endl;
		for(int j=0;j<LOOP;j++){
			cout << "---LOOP" << j << "---" << endl;
			measure.start();
			MusicManager musicManager;
			musicManager.setPropertiesManager(&properties);
			musicManager.loadMusicFromFile(fileName[i],musicLength[i]);
			musicManager.composeMusic();
			musicManager.saveMusic();
			musicManager.saveChord();
			measure.end();
			properties.setTime(measure.getDiff());
			properties.saveProperties();
		}
	}
	cout << "---Fin.---" << endl;
	cout << "Please push \"Enter\" key." << endl;
	getchar();
	return 0;
}