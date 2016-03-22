#include "stdafx.h"
#include "stdlib.h"
#include <iostream>
#include <fstream>
#include <cmath>
#include <algorithm>
#include <string>
#include <ctime>
#include "Helpers.h"
#include "Constants.h"

#include "Constructs.h"
using namespace std;


#pragma region AudioInfo // For info interfacing between input and algorithm

AudioInfo::AudioInfo(bool random) // argument defaults to false
{
	if (random)
	{
		_frequency = new double;
		_loudness = new double;
		_tempo = new double;
		randomize_info();
	}
	else
	{
		_frequency = NULL;
		_loudness = NULL;
		_tempo = NULL;
	}
}

AudioInfo::AudioInfo(double * freq, double * loudness, double * tempo)
{
	_frequency = new double;
	_loudness = new double;
	_tempo = new double;
	if (freq != NULL) { *_frequency = *freq; }
	if (loudness != NULL) { *_loudness = *loudness; }
	if (tempo != NULL) { *_tempo = *tempo; }
}

AudioInfo::~AudioInfo()
{
	_frequency = NULL;
	_loudness = NULL;
	_tempo = NULL;
	delete _frequency;
	delete _loudness;
	delete _tempo;
}

void AudioInfo::randomize_info()
{
	*_frequency = Helpers::fRand(FREQ_LB, FREQ_UB);
	*_loudness = Helpers::fRand(LOUD_LB, LOUD_UB);
	*_tempo = Helpers::fRand(TEMPO_LB, TEMPO_UB);
}

bool AudioInfo::operator == (const AudioInfo& b) const
{
	double freqa, louda, tempoa, freqb, loudb, tempob;
	bool a_has_no_nulls = this->get_frequency(freqa) && this->get_loudness(louda) && this->get_tempo(tempoa);
	bool b_has_no_nulls = b.get_frequency(freqb) && b.get_loudness(loudb) && b.get_tempo(tempob);
	bool a_equal_b = abs(freqa - freqb) <= AI_EQUAL_THRESH && abs(louda - loudb) <= AI_EQUAL_THRESH && abs(tempoa - tempob) <= AI_EQUAL_THRESH;
	return a_has_no_nulls && b_has_no_nulls && a_equal_b;
}

AudioInfo AudioInfo::differences(AudioInfo b)
{
	AudioInfo diffs;
	double freqa, louda, tempoa;
	double freqb, loudb, tempob;
	double freq, loud, tempo;

	if (this->get_frequency(freqa) && b.get_frequency(freqb))
	{
		freq = freqa - freqb;
		diffs.set_frequency(freq);
	}
	if (this->get_frequency(louda) && b.get_frequency(loudb))
	{
		loud = louda - loudb;
		diffs.get_loudness(loud);
	}
	if (this->get_frequency(tempoa) && b.get_frequency(tempob))
	{
		tempo = tempoa - tempob;
		diffs.get_tempo(tempo);
	}
	return diffs;
}


// Getters/setters
bool AudioInfo::get_frequency(double & freq) const
{
	if (_frequency != NULL) { freq = *_frequency; }
	return _frequency != NULL;
}
void AudioInfo::set_frequency(double freq)
{
	if (_frequency == NULL) _frequency = new double;
	if (freq > FREQ_UB)
		*_frequency = FREQ_UB;
	else if (freq < FREQ_LB)
		*_frequency = FREQ_LB;
	else
		*_frequency = freq;
}

bool AudioInfo::get_loudness(double & loud) const
{
	if (_loudness != NULL) { loud = *_loudness; }
	return _loudness != NULL;
}
void AudioInfo::set_loudness(double freq)
{
	if (_loudness == NULL) _loudness = new double;
	if (freq > LOUD_UB)
		*_loudness = LOUD_UB;
	else if (freq < LOUD_LB)
		*_loudness = LOUD_LB;
	else
		*_loudness = freq;
}

bool AudioInfo::get_tempo(double & tempo) const
{
	if (_tempo != NULL) { tempo = *_tempo; }
	return _tempo != NULL;
}
void AudioInfo::set_tempo(double freq)
{
	if (_tempo == NULL) _tempo = new double;
	if (freq > TEMPO_UB)
		*_tempo = TEMPO_UB;
	else if (freq < TEMPO_LB)
		*_tempo = TEMPO_LB;
	else
		*_tempo = freq;
}
void AudioInfo::clear_tempo() {
	_tempo = NULL;
}

#pragma endregion


#pragma region LightsInfo // For info interfacing between algorithm and output

LightsInfo::LightsInfo()
{
	red_intensity = 0;
	green_intensity = 0;
	blue_intensity = 0;
	white_intensity = 0;
	dimness = 0;
	strobing_speed = 0;
}

LightsInfo::LightsInfo(bool centered)
{
	if (centered)
	{
		red_intensity = (int)(R_UB - R_LB) / 2;
		green_intensity = (int)(G_UB - G_LB) / 2;
		blue_intensity = 0; //(int)(B_UB - B_LB) / 2;
		white_intensity = 0;
		dimness = 0; // (int)(DIM_UB - DIM_LB) / 2;
		strobing_speed = 0; // TODO: set strobing correctly
	}
	else
	{
		red_intensity = 0;
		green_intensity = 0;
		blue_intensity = 0;
		white_intensity = 0;
		dimness = 0;
		strobing_speed = 0;
	}
}

// MUST PASS ARRAY OF SIZE 513
unsigned char * LightsInfo::convert_to_output(unsigned char lightsOutput[])
{
	// Convert to 7-channel format
	lightsOutput[0] = 0;
	lightsOutput[1] = red_intensity;
	lightsOutput[2] = green_intensity;
	lightsOutput[3] = blue_intensity;
	lightsOutput[4] = white_intensity;
	lightsOutput[5] = 0;
	lightsOutput[6] = strobing_speed;
	lightsOutput[7] = dimness;

	return lightsOutput;
}

bool LightsInfo::operator == (const LightsInfo& b) const
{
	return abs((int)red_intensity - (int)b.red_intensity) <= LI_EQUAL_THRESH &&
		abs((int)blue_intensity - (int)b.blue_intensity) <= LI_EQUAL_THRESH &&
		abs((int)green_intensity - (int)b.green_intensity) <= LI_EQUAL_THRESH &&
		abs((int)white_intensity - (int)b.white_intensity) <= LI_EQUAL_THRESH &&
		abs((int)strobing_speed - (int)b.strobing_speed) <= LI_EQUAL_THRESH &&
		abs((int)dimness - (int)b.dimness) <= LI_EQUAL_THRESH;;
}

LightsInfo LightsInfo::average_and_smooth(deque<LightsInfo> outputs)
{
	LightsInfo avg = LightsInfo(false);
	int size = outputs.size();
	if (size == 0) return avg;

	// Average values
	for each (LightsInfo out in outputs)
	{
		avg.red_intensity += out.red_intensity;
		avg.green_intensity += out.green_intensity;
		avg.blue_intensity += out.blue_intensity;
		avg.white_intensity += out.white_intensity;
		avg.dimness += out.dimness;
		avg.strobing_speed += out.strobing_speed;
	}
	avg.red_intensity /= size;
	avg.green_intensity /= size;
	avg.blue_intensity /= size;
	avg.white_intensity /= size;
	avg.dimness /= size;
	avg.strobing_speed /= size;

	// Smooth final result
	if (avg.red_intensity < OUT_PARAM_TOO_SMALL_THRESH) avg.red_intensity = 0;
	if (avg.green_intensity < OUT_PARAM_TOO_SMALL_THRESH) avg.green_intensity = 0;
	if (avg.blue_intensity < OUT_PARAM_TOO_SMALL_THRESH) avg.blue_intensity = 0;
	if (avg.white_intensity < OUT_PARAM_TOO_SMALL_THRESH) avg.white_intensity = 0;

	return avg;
}

#pragma endregion


#pragma region SectionInfo // For sending section info from WavGen to OptiAlgo

SectionInfo::SectionInfo()
{
	section = verse;
	should_strobe = false;
}

SectionInfo::SectionInfo(SECTION section, bool should_strobe)
{
	this->section = section;
	this->should_strobe = should_strobe;
}

#pragma endregion