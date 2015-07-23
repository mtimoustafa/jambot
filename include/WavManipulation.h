#include <string.h>
#include <vector>
#include <deque>
#include "Helpers.h"

using namespace std;

class WavManipulation {

private:
	vector<short> durations;
	vector<string> filenames;
	int durationCounter;
	string directoryPath;
	vector<float> realTimeBuffer;

public: 
	WavManipulation();
	~WavManipulation();
	Helpers::SongStructure wavComparison(vector<float>);
	void snipAudio(vector<string>, vector<short>, vector<short>, string, string);
	void comparisonPolling();
	void startSnip();
	void realTimePush(vector<float>);
	void inputData(vector<float>);
};
