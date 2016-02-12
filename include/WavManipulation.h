#include <string.h>
#include <vector>
#include <deque>
#include "Helpers.h"
#include "soundfile.h"

using namespace std;

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
	SecAnlys();
	SecAnlys(string, vector<float>, int);
	~SecAnlys();
};
class WavManipulation {

private:
	vector<short> durations;
	vector<string> filenames;
	vector<SecAnlys> analysis;
	bool terminate;
	int durationCounter;
	string directoryPath;
	static vector<float> realTimeBuffer;
	static vector<SecAnlys> freqList;
	float hannFunction(int);
	float freqtonote(float);
	float notetofreq(int);
	float threshold(float);
	bool checkrepeats(string);


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
	void freqSnip(string, string, string);
	void dataStore(string, vector<SongSection>);
	void freqcomparison();
	static void inputData(vector<float>);
	float freqAnalysis(vector<float>);
	void startanalysis();
	void start();
	void stop();
	static bool pushFrequency(float);
	static bool readFrequency();
};
