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

AudioInfo::AudioInfo()
{
	_frequency = new double;
	_loudness = new double;
	_tempo = new double;
	randomize_info();
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

// Getters/setters
bool AudioInfo::get_frequency(double & freq) const
{
	if (_frequency != NULL) { freq = *_frequency; }
	return _frequency != NULL;
}
void AudioInfo::set_frequency(double freq)
{
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
	if (freq > TEMPO_UB)
		*_tempo = TEMPO_UB;
	else if (freq < TEMPO_LB)
		*_tempo = TEMPO_LB;
	else
		*_tempo = freq;
}

#pragma endregion


#pragma region LightsInfo // For info interfacing between algorithm and output

bool LightsInfo::operator == (const LightsInfo& b) const
{
	return abs((int)red_intensity - (int)b.red_intensity) <= LI_EQUAL_THRESH &&
		abs((int)blue_intensity - (int)b.blue_intensity) <= LI_EQUAL_THRESH &&
		abs((int)green_intensity - (int)b.green_intensity) <= LI_EQUAL_THRESH &&
		abs((int)white_intensity - (int)b.white_intensity) <= LI_EQUAL_THRESH &&
		abs((int)strobing_speed - (int)b.strobing_speed) <= LI_EQUAL_THRESH &&
		abs((int)dimness - (int)b.dimness) <= LI_EQUAL_THRESH;;
}

bool LightsInfo::is_similar_to(LightsInfo b)
{
	bool result = true;
	result = result && abs((int)red_intensity - (int)b.red_intensity) <= LI_SIMILAR_THRESH;
	result = result && abs((int)blue_intensity - (int)b.blue_intensity) <= LI_SIMILAR_THRESH;
	result = result && abs((int)green_intensity - (int)b.green_intensity) <= LI_SIMILAR_THRESH;
	result = result && abs((int)white_intensity - (int)b.white_intensity) <= LI_SIMILAR_THRESH;
	result = result && abs((int)strobing_speed - (int)b.strobing_speed) <= LI_SIMILAR_THRESH;
	result = result && abs((int)dimness - (int)b.dimness) <= LI_SIMILAR_THRESH;
	return result;
}

#pragma endregion