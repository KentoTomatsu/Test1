#pragma once
#include"common.h"
#include"Music.h"
#include"ChordManager.h"
#include"MusicChord.h"

const double THRESHOLD_RATE[FITNESS_LENGTH]={1,0.2,1,1};	
const double DISHARMONY_RATE=0.0;
//const double PLUS=-0.65;
const double PLUS=10;
const double MINUS=-1;

class Theorist
{
public:
	Theorist(void);
	~Theorist(void);
	void setBase(int ***base,int baseLength,int *baseMusicLength,int scale);
	void setMusicLength(int musicLength);
	void setPopSize(int popSize);
	void initParameters();
	void setGene(int ***gene);
	void setFitness(double *fitness,double **detailFitness,double **detailMusicTheory, double **detailGlobalSimilarity);
	void setMaxSimilarity();
	void setIndividual(int **individual);
	void calcFitness(bool executeLS);
	void musicTheory();
	void globalSimilarity();
	double chordSimilarity();
	double impulseSimilarity();
	void localSimilarity();
	void calcFitnessForIndividual(int **gene,int id);
	void musicTheoryForIndividual(int id);
	void globalSimilarityForIndividual(int id);
	double localSimilarityForIndividual();
	double getSuperordinateFitnessWeight(int id);
	double *getFitnessWeight();
	double getPitchRange();
	double *getImpulseRate();
	double getAverageImpulseRate();
	double getOverAverageImpulseRate();
	void calcBaseFitness();
	void printFitnessWeight();
	void printBaseFitness();
private:
	int ***genotype;	//å¬ëÃÇÃà‚ì`éqå^:key,rising
	int **individual;
	int musicLength;
	int popSize;
	double *fitness;
	double **detailFitness;
	double **detailMusicTheory;
	double **detailGlobalSimilarity;
	int ***baseGenotype;
	int **baseIndividual;
	int baseLength;//ÉxÅ[ÉXã»ÇÃêî
	int *baseMusicLength;
	int baseIndividualLength;
	double **baseDetailFitness;
	double *similarity;
	double maxGlobalSimilarity;
	double maxLocalSimilarity;
	MusicChord **baseChord;
	MusicChord **chords;
	MusicChord *chord;
	MusicChord *baseIndividualChord;
	int chordLength;
	int *series;
	int scale;
	//KEY tonic;
	int medianTone;
	int pattern[SCALE_LENGTH];
	bool rsCheck;
	double melodySD;
	double fitnessWeight[FITNESS_LENGTH];
	double maxFitnesses[FITNESS_LENGTH];
	double superordinateFitnessWeight[SUPERORDINATE_FITNESS_LENGTH];
	double impulseRate[BASE_NUM];
	double maxImpulseRate;
	double averageImpulseRate;
	double overAverageImpulseRate;
	double (Theorist::*globalSimilarityFunctions[GLOBAL_SIMILARITY_LENGTH])();
	double (Theorist::*fitnessFunctions[FITNESS_LENGTH])();
	double (Theorist::*fitnessForBaseFunctions[FITNESS_LENGTH])();
	ChordManager chordManager;
	MyRandom random;

	void setMedianTone();
	void setChord();
	int calcInterval(int prev,int next);
	double smooth();
	double harmony();
	double rhythmicContinuity();
	double standardDeviationOfMelody();
	double standardDeviationOfRhythm();
	double accentMelodyLine();
	double chordAdaptation();
	double smoothForBase();
	double harmonyForBase();
	double standardDeviationOfMelodyForBase();
	double accentMelodyLineForBase();
	double rhythmicContinuityForBase();
	double chordAdaptationForBase();
	double impulseSimilarityForBase();
	double returnZero();
	double returnMinus();
	double returnPlus();
	void setStandardDeviationOfMelodyForBase();
	void setImpulseRate();
	void setBaseChord();
	double serialSameToneForBase(int n);
	double selectSuperordinateFitness(SUPERORDINATE_FITNESS type);
	void setFitnessWeight();
	void setMaxFitnesses();
	void printChord();
	bool meetTheory();
};