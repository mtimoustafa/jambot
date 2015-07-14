#include <string.h>
#include <vector>

using namespace std;

class WavManipulation {

private:
	vector<short> durations;
	vector<string> filenames;
	int durationCounter;
	string directoryPath;

public: 
	WavManipulation();
	~WavManipulation();
	string wavComparison(short*);
	void snipAudio(vector<string>, vector<short>, vector<short>,string);


};
