// WavSnippet.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "soundfile.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctime>
#include <cmath>


#ifndef OLDCPP
#include <iostream>
using namespace std;
#else
#include <iostream.h>
#endif

class WavManipulation {

private:
	int durationTicks[20];


public : 

	static int wavComparison(int argc, char** argv) {
		char* inputname1 = "C:\\Users\\emerson\\Documents\\Visual Studio 2013\\Projects\\SoundSubTest\\SoundSubTest\\Debug\\crunchy_bass_swag.flv(1).wav";
		char* inputname2 = "C:\\Users\\emerson\\Documents\\Visual Studio 2013\\Projects\\SoundSubTest\\SoundSubTest\\Debug\\crunchy_bass_swag.flv.wav";
		char* outputname = "C:\\Users\\emerson\\Documents\\Visual Studio 2013\\Projects\\SoundSubTest\\SoundSubTest\\Debug\\Subtest1.wav";


		SoundFileRead  insound1(inputname1);
		SoundFileRead  insound2(inputname2);
		SoundFileWrite outsound(outputname, insound1);
		double exectime;
		time_t start = time(nullptr);

		int i, channel;
		int error_counter = 0;
		double threshold;
		double sample = 0.0;
		for (i = 0; i<insound1.getSamples(); i++) {
			for (channel = 0; channel < insound1.getChannels(); channel++) {
				sample = insound1.getCurrentSampleDouble(channel);
				sample = sample - insound2.getCurrentSampleDouble(channel);
				outsound.writeSampleDouble(sample);
			}
			insound1.incrementSample();
			insound2.incrementSample();
		}
		time_t end = time(nullptr);
		exectime = difftime(end, start);
		return 0;
	}

	void snipAudio(string names[], double startTimes[], double durationTimes[], char* filePath, int numofsnips){

		SoundFileRead insound(filePath);
		SoundHeader header = insound;
		int startSample = 0;
		int stopSample = 0;
		int n = 0;
		int numSnips = 0;


		for (int i = 0; i < numofsnips; i++){
			durationTicks[i] = ceil(durationTimes[i] / 0.2);
			string outName = names[i] + ".wav3";
			SoundFileWrite outsound(outName.c_str(), header);
			startSample = (int)(startTimes[i] * insound.getSrate() + 0.5);
			stopSample = (int)((startTimes[i] + 0.2) * insound.getSrate() + 0.5);
			n = stopSample - startSample;
			insound.gotoSample(startSample);
			for (int j = 0; j < n; j++){
				for (int k = 0; k < header.getChannels(); k++) {
					double sample = insound.getCurrentSampleDouble(k);
					outsound.writeSampleDouble(insound.getCurrentSampleDouble(k));
				}
				insound.incrementSample();
			}

		}
	}

};

int _tmain(int argc, _TCHAR* argv[])
{
	WavManipulation wav;
	double exectime;
	string name[1] = { "Chorus" };
	double start[1] = { 3.0 };
	int num = sizeof(start)/ sizeof(start[0]);
	double duration[1] = { 1.0 };
	char* file = "C:\\Users\\emerson\\Documents\\Visual Studio 2013\\Projects\\WavSnippet\\WavSnippet\\crunchy_bass_swag.flv.wav";
	time_t startTime = time(nullptr);
	wav.snipAudio(name, start, duration, file, num);
	time_t endTime = time(nullptr);
	exectime = difftime(endTime, startTime);
	return 0;
}

