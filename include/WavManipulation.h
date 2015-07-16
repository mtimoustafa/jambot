#include <string.h>
#include <vector>
#include "Helpers.h"

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
	Helpers::SongStructure wavComparison(short*);
	void snipAudio(vector<string>, vector<short>, vector<short>,string);


};
