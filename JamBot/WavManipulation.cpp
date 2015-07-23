// WavSnippet.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "soundfile.h"
#include "WavManipulation.h"
#include "Helpers.h"
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <ctime>
#include <vector>
#include <cmath>
#include <iostream>


//#ifndef OLDCPP
//#include <iostream>
//using namespace std;
//#else
//#include <iostream.h>
//#endif
using namespace std;

WavManipulation::WavManipulation(){
	durations = vector<short>();
	filenames = vector<string>();
	realTimeBuffer = vector<float>();
	durationCounter = 0;
	directoryPath = "";
}
WavManipulation::~WavManipulation(){
	durations.clear();
	filenames.clear();
	realTimeBuffer.clear();
	durationCounter = 0;
	directoryPath = "";
}

Helpers::SongElement getElement(string name){
	string temp = name.substr(0, name.length() - 4);
	Helpers::SongElement retElement = Helpers::NIL;
	if (temp == "Intro" || temp == "intro" || temp == "INTRO"){
		retElement = Helpers::INTRO;
	}
	if (temp == "Verse" || temp == "verse" || temp == "VERSE"){
		retElement = Helpers::VERSE;
	}
	if (temp == "Chorus" || temp == "chorus" || temp == "CHORUS"){
		retElement = Helpers::CHORUS;
	}
	if (temp == "Bridge" || temp == "bridge" || temp == "BRIDGE"){
		retElement = Helpers::INTRO;
	}
	if (temp == "Outro" || temp == "outro" || temp == "OUTRO"){
		retElement = Helpers::OUTRO;
	}
	return retElement;
}
int getNum(string name){
	string temp = name.substr(name.length() - 5, 1);
	int num = 0;
	try{
		num = std::stoi(temp.c_str());
	}
	catch (invalid_argument e){
		num = 0;
	}
	return num;
}
void WavManipulation::inputData(vector<float> buffer){
	realTimeBuffer = buffer;
}
///////////////////////////////////////////
///	wavComparison: Compares a wav file with realtime input
///					Runs until it finds a match or hits the end of the files
/// Input: short realTimeBuffer[] - the buffered values of the real time input
///		   string - directoryPath - the directory path name
/// Output: char*

Helpers::SongStructure WavManipulation::wavComparison(vector<float> data) {

	unsigned short i, j, channel;
	int error_counter = 1000;
	short threshold = 0.0009;
	Helpers::SongElement element = Helpers::NIL;
	int num = 0;
	float sample = 0;

	for (j = 0; j < filenames.size(); j++){
		short k = 0;
		string readFile = directoryPath + filenames[j];
		SoundFileRead  insound1(readFile.c_str());
		durationCounter = durations[j];
		for (i = 0; i < insound1.getSamples(); i++) {
			if (error_counter == 0){
				break;
			}
			for (channel = 0; channel < insound1.getChannels(); channel++) {
				sample = (float)insound1.getCurrentSampleDouble(channel);
				sample = sample - realTimeBuffer[k + (channel)];
				if (abs(sample) > threshold){
					error_counter--;
				}
				else{
					error_counter++;
				}
			}
			k += 2;
			insound1.incrementSample();
		}
		if (error_counter > 0){
			element = getElement(filenames[i]);
			num = getNum(filenames[i]);
			return Helpers::SongStructure(element, num);
		}
	}
	return Helpers::SongStructure(element, 0);
}
///////////////////////////////////////////
///	snipAudio: Snips the audio file into sections, all wav files using the names in names vector
/// Input: vector: names - The names of each sections in order of the time they appear in the song
///		   vector: startTimes - The times (in seconds) of each section
///		   vector: durationTimes - The length (in seconds) of each section
///		   filepath - The path of the file being cut
/// Output: VOID

void WavManipulation::snipAudio(vector<string> names, vector<short> startTimes, vector<short> durationTimes, string filePath, string filename){

	SoundFileRead insound((filePath + filename).c_str());
	SoundHeader header = insound;
	int startSample = 0;
	int stopSample = 0;
	int n = 0;
	int numSnips = 0;

	for (unsigned int i = 0; i < startTimes.size(); i++){
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
				outsound.writeSampleDouble(insound.getCurrentSampleDouble(k));
			}
			insound.incrementSample();
		}

	}
}

void WavManipulation::comparisonPolling(){
	//Queue pull
	vector<float> indata; // data->recordedBuffer.begin();
	std::deque<Helpers::SongStructure>  structurequeue;
	//
	while (true){
		if (realTimeBuffer.empty()){
			continue;
		}
		if (durationCounter > 0){
			durationCounter--;
		}
		else{
			indata = realTimeBuffer;
			Helpers::SongStructure section = wavComparison(indata);
			structurequeue.push_back(section); //This needs to be adjusted to push to mohammed's queue
		}
	}
}

void WavManipulation::startSnip(){
	string filename = "crunchy_bass_swag.flv";
	string filepath = "C:\\Users\\emerson\\Documents\\School\\FYDP\\jambot\\JamBot\\";
	vector<string> names = {"Intro","Chorus1","Verse1", "Outro"};
	vector<short> durations = {1,1,1,1};
	vector<short> startTimes = {1,3,5,7};
	snipAudio(names, startTimes, durations, filepath, filename);
}