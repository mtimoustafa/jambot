#include <string.h>
#include <vector>
#include <deque>
#include "Helpers.h"
#include "soundfile.h"

using namespace std;

class WavManipulation {

private:
	vector<short> durations;
	vector<string> filenames;
	int durationCounter;
	string directoryPath;
	static vector<float> realTimeBuffer;

public: 
	WavManipulation();
	~WavManipulation();
	Helpers::SongStructure wavComparison();
	void snipAudio(vector<string>, vector<short>, vector<short>, string, string);
	void comparisonPolling();
	void startSnip();
	void realTimePush(vector<float>);
	static void inputData(vector<float>);
};
