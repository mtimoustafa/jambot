// WavSnippet.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "soundfile.h"
#include "WavManipulation.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctime>
#include <vector>
#include <cmath>


#ifndef OLDCPP
#include <iostream>
using namespace std;
#else
#include <iostream.h>
#endif

//Vector is the best option


WavManipulation::WavManipulation(){
	durations = vector<short>();
	filenames = vector<string>();
	durationCounter = 0;
	directoryPath = "";
}
WavManipulation::~WavManipulation(){

}
///////////////////////////////////////////
///	wavComparison: Compares a wav file with realtime input
///					Runs until it finds a match or hits the end of the files
/// Input: short realTimeBuffer[] - the buffered values of the real time input
///		   string - directoryPath - the directory path name
/// Output: char*

string WavManipulation::wavComparison(short* realTimeBuffer) {

	int i, j, channel;
	int error_counter = 0;
	short threshold = 100;

	short sample = 0;
	if (durationCounter > 0){
		durationCounter--;
		return "";
	}
	for (j = 0; j < filenames.size(); j++){
		string readFile = directoryPath + filenames[j];
		SoundFileRead  insound1(readFile.c_str());
		for (i = 0; i < insound1.getSamples(); i++) {
			if (error_counter == 0){
				durationCounter = durations[j];
				break;
			}
			for (channel = 0; channel < insound1.getChannels(); channel++) {
				sample = insound1.getCurrentSample16Bit(channel);
				sample = sample - realTimeBuffer[i];
				if (sample > threshold){
					error_counter--;
				}
				else{
					error_counter++;
				}
			}
			insound1.incrementSample();
		}
		if (error_counter > 0){
			return filenames[j];
		}
	}
	return "No Equal Found";
}
///////////////////////////////////////////
///	snipAudio: Snips the audio file into sections, all wav files using the names in names vector
/// Input: vector: names - The names of each sections in order of the time they appear in the song
///		   vector: startTimes - The times (in seconds) of each section
///		   vector: durationTimes - The length (in seconds) of each section
///		   filepath - The path of the file being cut
/// Output: VOID

void WavManipulation::snipAudio(vector<string> names, vector<short> startTimes, vector<short> durationTimes, string filePath){

	SoundFileRead insound(filePath.c_str());
	SoundHeader header = insound;
	int startSample = 0;
	int stopSample = 0;
	int n = 0;
	int numSnips = 0;

	for (int i = 0; i < startTimes.size(); i++){
		durations.push_back((short)ceil(durationTimes[i] / 0.2));  //store durations as a number of 200ms bursts
		string outName = names[i] + ".wav";
		filenames.push_back(names[i]); //store file names for the sections
		SoundFileWrite outsound(outName.c_str(), header);
		startSample = (short)(startTimes[i] * insound.getSrate() + 0.5);  //starting sample
		stopSample = (short)((startTimes[i] + 0.2) * insound.getSrate() + 0.5);//ending sample
		n = stopSample - startSample; //number of samples
		insound.gotoSample(startSample);
		for (int j = 0; j < n; j++){
			for (int k = 0; k < header.getChannels(); k++) { //for each channel of each sample
				//write the sample from original to current file
				outsound.writeSample16Bit(insound.getCurrentSample16Bit(k));
			}
			insound.incrementSample();
		}

	}
}
