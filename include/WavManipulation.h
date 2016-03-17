#include "stdafx.h"
#include <string.h>
#include <vector>
#include <deque>
#include "Helpers.h"
#include "soundfile.h"

using namespace std;
class SectionLyric{
public:
	string lyric;
	int numLines;
	SectionLyric(string, int);
	~SectionLyric();
};

class SongSection{
public:
	string Name;
	double startTime;
	SongSection(string, double);
	~SongSection();
};
class SecAnlys{
public:
	string name;
	vector<float> pitch;
	int duration;
	string lyric;
	SecAnlys();
	SecAnlys(string, vector<float>, int);
	SecAnlys(string, string);
	~SecAnlys();
};
class WavManipulation {

private:
	vector<short> durations;
	vector<string> filenames;
	vector<SecAnlys> analysis;
	//queue<float> input;
	//queue<string> section;
	//bool terminate;
	int durationCounter;
	string directoryPath;
	static vector<float> realTimeBuffer;
	static vector<SecAnlys> freqList;
	float hannFunction(int);
	float freqtonote(float);
	float notetofreq(int);
	float threshold(float);
	bool checkrepeats(string);
	static vector<SecAnlys> lyrics;
	bool checklyricrepeats(string);


public:
	WavManipulation();
	~WavManipulation();
	void wavComparison();
	void snipAudio(vector<string>, vector<double>, vector<short>, string, string);
	void setFrequency(float);
	void freqAnalysis();
	void comparisonPolling();
	void startSnip();
	void realTimePush(vector<float>);
	void freqSnip(string);
	void dataStore(string, vector<SongSection>, string, string);
	void freqcomparison();
	static void inputData(vector<float>);
	float freqAnalysis(vector<float>);
	void startanalysis();
	void start(string);
	void stop();
	void parseTxt(string);
	static bool pushFrequency(float);
	static void startReading(bool);
	static bool readFrequency();
};
