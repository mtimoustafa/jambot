
class WavManipulation {

private:
	vector<short> durations;
	vector<string> filenames;
	int durationCounter;
	string directoryPath;

public: 
	WavManipulation(void);
	~WavManipulation();
	string wavComparison(short[]);
	void snipAudio(vector<string>, vector<short>, vector<short>,string);


};
