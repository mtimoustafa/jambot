// WavSnippet.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "soundfile.h"
#include "WavManipulation.h"
#include "InputChannelReader.h"
#include "Helpers.h"
#include "Constants.h"
#include "fftw3.h"
#include "JamBot.h"
#include "OptiAlgo.h"
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <ctime>
#include <vector>
#include <cmath>
#include <iostream>
#include <fstream>
#include <sstream>
#include <math.h>
#include <cmath>
#include <queue>
#include <algorithm>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define NUM_FREQ 3
//#ifndef OLDCPP
//#include <iostream>
//using namespace std;
//#else
//#include <iostream.h>
//#endif
using namespace std;

vector<float> WavManipulation::realTimeBuffer = vector<float>();
vector<SecAnlys> WavManipulation::freqList = vector<SecAnlys>();
vector<SecAnlys> WavManipulation::lyrics = vector<SecAnlys>();
static queue<float> frequency = queue<float>();
static queue<string> section = queue<string>();
static bool checkfrequency;

SongSection::SongSection(string n, double t){
	Name = n;
	startTime = t;
}
SongSection::~SongSection(){
}
SecAnlys::SecAnlys(){
}
SecAnlys::SecAnlys(string n, vector<float> t, int d){
	name = n;
	pitch = t;
	duration = d;
}
SecAnlys::SecAnlys(string n, string l){
	name = n;
	lyric = l;
}
SecAnlys::~SecAnlys(){
	pitch.clear();
}


WavManipulation::WavManipulation(){
	durations = vector<short>();
	filenames = vector<string>();
	frequency = queue<float>();
	section = queue<string>();
	checkfrequency = false;
	terminate = false;
	durationCounter = 0;
	directoryPath = "";
}
WavManipulation::~WavManipulation(){
	durations.clear();
	filenames.clear();
	freqList.clear();
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
		retElement = Helpers::BRIDGE;
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
void WavManipulation::wavComparison() {

	unsigned short i, j, channel;
	int error_counter = 1000;
	double threshold = 0.0009;
	Helpers::SongElement element = Helpers::NIL;
	int num = 0;
	float sample = 0;

	for (j = 0; j < filenames.size(); j++){
		short k = 0;
		string readFile = "C:\\Users\\emerson\\Documents\\School\\FYDP\\jambot\\JamBot\\" + filenames[j] + ".wav";
		SoundFileRead  insound1(readFile.c_str());
		SoundFileRead  insound2("C:\\Users\\emerson\\Documents\\School\\FYDP\\jambot\\JamBot\\Verse3.wav");
		//durationCounter = durations[j];
		for (i = 0; i < insound1.getSamples(); i++) {
			if (error_counter == 0){
				break;
			}
			//for (channel = 0; channel < insound1.getChannels(); channel++) {
				sample = insound1.getCurrentSampleDouble(0);
				sample = sample - insound2.getCurrentSampleDouble(0);
				if (abs(sample) > threshold){
					error_counter--;
				}
				else{
					error_counter++;
				}
			//}
			k += 2;
			insound1.incrementSample();
		}
		if (error_counter > 0){
			//element = getElement(filenames[j]);
			//num = getNum(filenames[j]);
			//return Helpers::SongStructure(element, num);
			//Helpers::print_debug(filenames[j].c_str());
			//Helpers::print_debug("\n");
		}
	}
	//return Helpers::SongStructure(element, 0);
}
///////////////////////////////////////////
///	snipAudio: Snips the audio file into sections, all wav files using the names in names vector
/// Input: vector: names - The names of each sections in order of the time they appear in the song
///		   vector: startTimes - The times (in seconds) of each section
///		   vector: durationTimes - The length (in seconds) of each section
///		   filepath - The path of the file being cut
/// Output: VOID
void WavManipulation::snipAudio(vector<string> names, vector<double> startTimes, vector<short> durationTimes, string filePath, string filename){

	SoundFileRead insound((filePath + filename).c_str());
	SoundHeader header = insound;
	int startSample = 0;
	int stopSample = 0;
	int n = 0;
	int numSnips = 0;

	for (unsigned int i = 0; i < startTimes.size(); i++){
		//durations.push_back((short)ceil(durationTimes[i] / 0.2));  //store durations as a number of 200ms bursts
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
	vector<float> indata;
	//std::deque<Helpers::SongStructure>  structurequeue;
	//
	while (true){
		if (realTimeBuffer.empty()){
			continue;
		}
		if (durationCounter > 0){
			durationCounter--;
		}
		else{
			//Helpers::SongStructure section = wavComparison();
			//structurequeue.push_back(section); //This needs to be adjusted to push to mohammed's queue
		}
	}
}

void WavManipulation::startSnip(){
	string filename = "Boston_More_than_a_FeelingVocals_Only.wav";
	string filepath = "C:\\Users\\emerson\\Downloads\\";
	vector<string> names = { "Verse1", "Chorus", "Verse2", "Chorus", "Verse3", "Chorus" };
	vector<short> durations = { 1, 1, 1, 1 };
	vector<double> startTimes = { 23.0, 55.0, 90.0, 123.08, 187.4, 239.95 };
	snipAudio(names, startTimes, durations, filepath, filename);
}


///////////////////////////////////
//	Better Code
//////////////////////////////////
//This is where the frequency code is

void clearqueue(){
	queue<float> empty;
	swap(frequency, empty);
}
//This is a testing function for now
void WavManipulation::startanalysis(){
	vector<SongSection> secs = vector<SongSection>();
	//time_t startTime;
	//time_t endTime;
	//double exectime;
	//VCC
	//secs.push_back(SongSection("Verse", 0.8));
	//secs.push_back(SongSection("Chorus1", 25.2));
	//secs.push_back(SongSection("Chorus2", 53.4));
	//dataStore("Hey Jude VCC", secs, "C:\\Users\\emerson\\Documents\\School\\FYDP\\Sample Music\\Hey Jude VCC.wav",
	//	"C:\\Users\\emerson\\Documents\\School\\FYDP\\Sample Music\\Hey Jude VCC.txt");
	//freqSnip("Hey Jude VCC.csv");
	//VVC
	secs.push_back(SongSection("Verse1", 0.8));
	secs.push_back(SongSection("Verse2", 24));
	secs.push_back(SongSection("Chorus", 47.8));	
	dataStore("Hey Jude VVC", secs, "C:\\Users\\emerson\\Documents\\School\\FYDP\\Sample Music\\Hey Jude VVC.wav",
		"C:\\Users\\emerson\\Documents\\School\\FYDP\\Sample Music\\Hey Jude VVC.txt");
	freqSnip("Hey Jude VVC.csv");
	//VCV
	//secs.push_back(SongSection("Verse1", 0.8));
	//secs.push_back(SongSection("Chorus", 24.8));
	//secs.push_back(SongSection("Verse2", 53.6));
	//dataStore("Hey Jude VCV", secs, "C:\\Users\\emerson\\Documents\\School\\FYDP\\Sample Music\\Hey Jude VCV.wav", 
	//	"C:\\Users\\emerson\\Documents\\School\\FYDP\\Sample Music\\Hey Jude VCV.txt");
	//freqSnip("Hey Jude VCV.csv");
	start();
	//parseTxt("testlyrics");
}
/////////////////////////////////////////
///		Frequency Fingerprint Functions
//Brandon Function that is used to push the frequencies
bool WavManipulation::pushFrequency(float in){
	if (frequency.size() < AUDIO_BUF_SIZE)
		frequency.push(in);
	return frequency.size() < AUDIO_BUF_SIZE;
}

void WavManipulation::startReading(bool in){
	checkfrequency = in;
}

//dataStore(string, vector<SongSection>)
//Parameters filename: Name of the csv file probably just the name of the song
//			sections: a vector of the section information using the SongSection class
//This function takes the name of th csv file and the list of sections and writes them to a csv file from the GUI
void WavManipulation::dataStore(string csvname, vector<SongSection> sections, string wavfile, string lyricfile){
	ofstream song;
	song.open("CSV\\" + csvname + ".csv");
	song << "Wav File, Lyric File\n";
	song << wavfile + "," + lyricfile + "\n";
	song << "Section Name, Start Time\n";
	string str = "";
	for (int i = 0; i < sections.size(); i++){
		song << sections[i].Name + "," + to_string(sections[i].startTime) + "\n";
	}
	song.close();
}

bool WavManipulation::checkrepeats(string name){
	for (int i = 0; i < freqList.size(); i++){
		if (freqList[i].name == name){
			return true;
		}
	}
	return false;
}
float WavManipulation::freqtonote(float in){
	return 12 * log2f(in / 440) + 49;
}
float WavManipulation::notetofreq(int in){
	return pow((double)2, (double)(((double)in - (double)49) / (double)12)) * 440;
}
float WavManipulation::threshold(float in){
	int note = round((double)freqtonote(in));
	float lower = abs((in - notetofreq(note - 1)) / 2);
	float upper = abs((notetofreq(note + 1) - in) / 2);
	float thresh = (upper + lower);
	return thresh;
}
float WavManipulation::hannFunction(int n)
{
	double inner = (2 * M_PI * n) / (FFT_SIZE - 1);
	return (float)(0.5 * (1.0 - cos(inner)));
}

string lowercase(string str){
	std::transform(str.begin(), str.end(), str.begin(), ::tolower);
	return str;
}
float WavManipulation::freqAnalysis(vector<float> data){

	float *in;
	fftwf_complex *out;
	fftwf_plan frequencyPlan;
	InputChannelReader inchannel;
	float magnitude;
	short fileSamples;
	float maxDensity = 0.0;
	float freq;
	int maxIndex;
	in = (float*)fftwf_malloc(sizeof(float)* FFT_SIZE);
	out = (fftwf_complex*)fftwf_malloc(sizeof(fftwf_complex)* OUTPUT_SIZE);
	frequencyPlan = fftwf_plan_dft_r2c_1d(FFT_SIZE, in, out, FFTW_ESTIMATE);
	for (int i = 0; i < data.size(); i++)
	{
		// Use every-other sample
		if (i < FFT_SIZE)
		{
			in[i] = data[i] * hannFunction(i);
		}

	}
	// Get frequency of wave
	fftwf_execute(frequencyPlan);

	for (int i = 0; i < 605; i++)
	{
		magnitude = (float)sqrt(pow(out[i][0], 2) + pow(out[i][1], 2));

			if (magnitude > maxDensity)
			{
				maxDensity = magnitude;
				maxIndex = i;
			}
	}
	freq = maxIndex * SAMPLE_RATE / FFT_SIZE;
	fftwf_free(in);
	fftwf_free(out);
	return freq;
}
void WavManipulation::freqcomparison(){
	float inFreq = 0.0;
	float diff1 = 0.0;
	float diff2 = 0.0;
	int choruscount = 0;
	int versecount = 0;
	int c = -1;
	int v = -1;
	int chorusdiffcount = 0;
	int versediffcount = 0;
	int j = 0;
	int ticks = 0;
	int duration = 0;
	int noseccount = 0;
	vector<float> chorus, verse, chorusdiff, versediff, freqdiff, freq, cdiff, vdiff;
	vector<int> sectioncount;
	char * err_str;
	Helpers::SongElement element = Helpers::NIL;

	if (freqList.empty()){
		Helpers::print_debug("No song Selected or Analysed, Run freqSnip before this function");
		return;
	}
	for (int k = 0; k < freqList.size(); k++){
		if (lowercase(freqList[k].name).find("chorus") != string::npos){
			chorus = freqList[k].pitch;
			break;
		}
	}
	for (int k = 0; k < freqList.size(); k++){
		if (lowercase(freqList[k].name).find("verse") != string::npos){
			verse = freqList[k].pitch;
			break;
		}
	}
	for (int y = 0; y < chorus.size() - 1; y++){
		chorusdiff.push_back((chorus[y+1] - chorus[y]));
	}

	for (int y = 0; y < verse.size() - 1; y++){
		versediff.push_back((verse[y + 1] - verse[y]));
	}


	while (!terminate){
		while (ticks < duration || !checkfrequency){
			if (!frequency.empty())
			{
				ticks++;
				frequency.pop();
				//clearqueue();
			}
			//checkfrequency = false;
		}
		//clearqueue();
		//checkfrequency = true;
		try{
			while (j < NUM_FREQ){
				while (frequency.empty()){ 
					if (terminate){ 
						return; 
					} 
				}
				if (inFreq != 0.0)
					freqdiff.push_back((frequency.front() -  inFreq));

				inFreq = frequency.front();
				freq.push_back(inFreq);
				frequency.pop();

				diff1 = abs(inFreq - chorus[j]);
				diff2 = abs(inFreq - verse[j]);

				cdiff.push_back(diff1);
				vdiff.push_back(diff2);

				if (diff1 < diff2){
					choruscount++;
				}
				else if (diff2 < diff1){
					versecount++;
				}
				else
					

				ticks++;
				j++;
			}

			j = 0;
			for (int i = 0; i < NUM_FREQ - 1; i++){
				
				if (abs(chorusdiff[i] - freqdiff[i]) 
					< abs(versediff[i] - freqdiff[i]))
					choruscount++;
				else
					versecount++;
			}
			freqdiff.clear();
			freq.clear();
			cdiff.clear();
			vdiff.clear();
   			if (choruscount > versecount){
				for (int k = 0; k < freqList.size(); k++){
					if (lowercase(freqList[k].name).find("chorus") != string::npos){
						if (k != c){
							c = k; 
							break;
						}
					}
				}
				for (int k = 0; k < freqList.size(); k++){
					if (lowercase(freqList[k].name).find("chorus") != string::npos){
						if (k != c){
							chorus = freqList[k].pitch;	
							for (int y = 0; y < chorus.size() - 1; y++){
								chorusdiff[y] = (chorus[y + 1] - chorus[y]);
							}
							break;
						}
					}
				}
				for (int k = 0; k < lyrics.size(); k++){
					if (lowercase(lyrics[k].name).find(lowercase(freqList[c].name)) != string::npos){
						JamBot::updateLyrics(lyrics[k].lyric);
						break;
					}
				}
				OptiAlgo::receive_song_section(SECTION::chorus);
				Helpers::print_debug(freqList[c].name.c_str());
				Helpers::print_debug("\n");
				ticks = 0;
				duration = freqList[c].duration - 3 - (3*noseccount);
				noseccount = 0;
			}
			else if (versecount > choruscount){
				for (int k = 0; k < freqList.size(); k++){ 
					if (lowercase(freqList[k].name).find("verse") != string::npos){
						if (k != v){
							v = k;
							break;
						}
					}
				}			
				for (int k = 0; k < freqList.size(); k++){
					if (lowercase(freqList[k].name).find("verse") != string::npos){
						if (k != v){
							verse = freqList[k].pitch;
							for (int y = 0; y < verse.size() - 1; y++){
								versediff[y] = (verse[y + 1] - verse[y]);
							}
							break;
						}
					}
				}
				for (int k = 0; k < lyrics.size(); k++){
					if (lowercase(lyrics[k].name).find(lowercase(freqList[v].name)) != string::npos){
						JamBot::updateLyrics(lyrics[k].lyric);
						break;
					}
				}
				OptiAlgo::receive_song_section(SECTION::verse);
				Helpers::print_debug(freqList[v].name.c_str());
				Helpers::print_debug("\n");
				ticks = 0;
				duration = freqList[v].duration - 3 - (3 * noseccount);
				noseccount = 0;
			}
			else{
				Helpers::print_debug("No Section Found");
				Helpers::print_debug("\n");
				noseccount++;
			}
			choruscount = versecount = 0;
			inFreq = 0.0;
		}
		catch (exception e){
			err_str = "";
			strcat(err_str, "ERROR: WavManipulation: ");
			strcat(err_str, e.what());
			strcat(err_str, "\n");
			Helpers::print_debug(err_str);
		}

	}	
	if (terminate) Helpers::print_debug("WavManipulation: terminated.\n");
	else Helpers::print_debug("WavManipulation: stopped.\n");
}
bool WavManipulation::checklyricrepeats(string name){
	for (int i = 0; i < lyrics.size(); i++){
		if (lowercase(lyrics[i].name) == lowercase(name)){
			return true;
		}
	}
	return false;
}
void WavManipulation::parseTxt(string filename){
	ifstream file(filename);
	string value;
	SecAnlys section;
	getline(file, value, '[');
	while (!file.eof()){
		getline(file, value, ']');
		checklyricrepeats(value);
		section.name = value;
		getline(file, value, '[');
		section.lyric = value;
		lyrics.push_back(section);
	}
	file.close();
}

//freqSnip(string, string, string)
//Parameters: csvname: the name of the CSV file, The wav file and lyric file are stored in the csv file
//This function is meant to be called before live play is started, this collects the data 
//from the wav file to be compared to while live playing
void WavManipulation::freqSnip(string csvname){
	ifstream file("CSV\\" + csvname);
	vector<string> names;
	vector<double> times;
	string value = "";
	string wavfile = "";
	string lyricfile = "";
	bool even = true;
	getline(file, value, ',');
	getline(file, value, '\n');
	getline(file, value, ',');
	wavfile = value;
	getline(file, value, '\n');
	lyricfile = value;

	getline(file, value, ',');
	getline(file, value, '\n');
	while (!file.eof()){

		if (even){
			getline(file, value, ',');
			names.push_back(value);
		}
		else{
			getline(file, value, '\n');
			times.push_back(stod(value));
		}
		cout << value << endl;
		even = !even;
	}
	file.close();
	SoundFileRead insound((wavfile).c_str());
	SoundHeader header = insound;
	int startSample = 0;
	int stopSample = 0;
	int n = 0;
	int numSnips = 0;
	SecAnlys section;
	int length = floor((double)insound.getSamples() / (double)insound.getSrate());

	double freq = 0;
	for (unsigned int i = 0; i < times.size(); i++){
		vector<float> list;
		for (int l = 0; l < NUM_FREQ; l++){
			vector<float> snippet;
			startSample = round((times[i] + (l * 0.2)) * insound.getSrate() + 0.5);  //starting sample
			stopSample = round((times[i] + (l * 0.2) + 0.2) * insound.getSrate() + 0.5);//ending sample
			n = stopSample - startSample; //number of samples
			insound.gotoSample(startSample);
			for (int j = 0; j < n; j++){
				//for (int k = 0; k < insound.getChannels(); k++){
					snippet.push_back((int)(insound.getCurrentSampleDouble(0) * 0x8000));
					insound.incrementSample();
			}
			freq = freqAnalysis(snippet);
			if (freq >= 2000.0){
				freq = freqAnalysis(snippet);
			}
			list.push_back(freq);
			snippet.clear();
		}

		if (i < times.size() - 1){
			section = SecAnlys(names[i], list, ceil((times[i + 1] - times[i]) / 0.2));
		}
		else {
			section = SecAnlys(names[i], list, ceil((length - times[i]) / 0.2));
		}
		if (!checkrepeats(names[i])){
			//Helpers::print_debug(names[i].c_str());
			freqList.push_back(section);
		}
	}
	parseTxt(lyricfile);
	
}
void WavManipulation::start(string fileName){
	freqSnip(fileName);
	freqcomparison();
}
void WavManipulation::stop(){
	terminate = true;
}