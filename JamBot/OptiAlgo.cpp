#include "stdafx.h"
#include "stdlib.h"
#include <iostream>
#include <fstream>
#include <cmath>
#include <algorithm>
#include <list>
#include <map>
#include <vector>
#include <array>
#include <string>
#include <ctime>
#include <sstream>
#include <queue>
#include <exception>
#include <functional>

#include "Constructs.h"
#include "Constants.h"
#include "Helpers.h"
#include "OptiAlgo.h"
#include "DMXOutput.h"


using namespace std;


#pragma region AudioProps

OptiAlgo::AudioProps::AudioProps()
{
	freq_avg = 0.0;
	tempo_avg = 0.0;
	loudness_avg = 0.0;
	loudness_nomax_avg = 0.0;
	loudness_max_avg = 0.0;
}

double OptiAlgo::AudioProps::freq_add_and_avg(double val)
{
	double avg = 0.0;
	freq_hist.push_front(val);
	if (freq_hist.size() > HISTORY_BUF_SIZE) freq_hist.pop_back();
	for each (double val in freq_hist)
		avg += val;
	avg /= (double)freq_hist.size();
	freq_avg = avg;
	return avg;
}
double OptiAlgo::AudioProps::tempo_add_and_avg(double val)
{
	double avg = 0.0;
	tempo_hist.push_front(val);
	if (tempo_hist.size() > HISTORY_BUF_SIZE) tempo_hist.pop_back();
	for each (double val in tempo_hist)
		avg += val;
	avg /= (double)tempo_hist.size();
	tempo_avg = avg;
	return avg;
}
double OptiAlgo::AudioProps::loudness_hist_add_and_avg(double val)
{
	double avg = 0.0;
	loudness_hist.push_front(val);
	if (loudness_hist.size() > HISTORY_BUF_SIZE) loudness_hist.pop_back();
	for each (double val in loudness_hist)
		avg += val;
	avg /= (double)loudness_hist.size();
	loudness_avg = avg;
	return avg;
}
double OptiAlgo::AudioProps::loudness_nomax_hist_add_and_avg(double val)
{
	double avg = 0.0;
	loudness_nomax_hist.push_front(val);
	if (loudness_nomax_hist.size() > HISTORY_BUF_SIZE) loudness_nomax_hist.pop_back();
	for each (double val in loudness_nomax_hist)
		avg += val;
	avg /= (double)loudness_nomax_hist.size();
	loudness_nomax_avg = avg;
	return avg;
}
double OptiAlgo::AudioProps::loudness_max_hist_add_and_avg(double val)
{
	double avg = 0.0;
	loudness_max_hist.push_front(val);
	if (loudness_max_hist.size() > HISTORY_BUF_SIZE) loudness_max_hist.pop_back();
	for each (double val in loudness_max_hist)
		avg += val;
	avg /= (double)loudness_max_hist.size();
	loudness_max_avg = avg;
	return avg;
}

#pragma endregion

#pragma region FuzzyLogic

OptiAlgo::FLSystem::FLSystem() {
	// Initialize cutoffs
	ResetCutoffs();
}

void OptiAlgo::FLSystem::ResetCutoffs() {
	Rcutoff[none] = 1.0;
	Gcutoff[none] = 1.0;
	Bcutoff[none] = 1.0;

	Rcutoff[dark] = 1.0;
	Gcutoff[dark] = 1.0;
	Bcutoff[dark] = 1.0;

	Rcutoff[medium] = 1.0;
	Gcutoff[medium] = 1.0;
	Bcutoff[medium] = 1.0;

	Rcutoff[strong] = 1.0;
	Gcutoff[strong] = 1.0;
	Bcutoff[strong] = 1.0;

	Wcutoff[off] = 1.0;
	Wcutoff[normal] = 1.0;
	Wcutoff[bright] = 1.0;
}

#pragma region MembershipFunctions

double OptiAlgo::FLSystem::FreqInClass(double input, FreqClassIDs flClass) {
	double mu;
	switch (flClass) {
	case vlow:
		if (input <= 200.0) mu = 1.0;
		else if (input < 250.0) mu = 1.0 - (input - 200.0) / (250.0 - 200.0);
		else mu = 0.0;
		break;
	case low:
		if (input <= 190.0) mu = 0.0;
		else if (input <= 300.0) mu = (input - 190.0) / (300.0 - 190.0);
		else if (input < 420.0) mu = 1.0 - (input - 300.0) / (420.0 - 300.0);
		else mu = 0.0;
		break;
	case high:
		if (input <= 320.0) mu = 0.0;
		else if (input <= 450.0) mu = (input - 320.0) / (450.0 - 320.0);
		else if (input < 580.0) mu = 1.0 - (input - 450.0) / (580.0 - 450.0);
		else mu = 0.0;
		break;
	case vhigh:
		if (input <= 480.0) mu = 0.0;
		else if (input < 600.0) mu = (input - 480.0) / (600.0 - 480.0);
		else mu = 1.0;
		break;
	default:
		Helpers::print_debug("ERROR: undefined freq class requested.\n");
		mu = 0.0;
		break;
	}
	return mu;
}

double OptiAlgo::FLSystem::TempoInClass(double input, TempoClassIDs flClass) {
	double mu;
	switch (flClass) {
	case vslow:
		if (input <= 30.0) mu = 1.0;
		else if (input < 50.0) mu = 1.0 - (input - 30.0) / (50.0 - 30.0);
		else mu = 0.0;
		break;
	case slow:
		if (input <= 35.0) mu = 0.0;
		else if (input <= 50.0) mu = (input - 35.0) / (50.0 - 35.0);
		else if (input < 70.0) mu = 1.0 - (input - 50.0) / (70.0 - 50.0);
		else mu = 0.0;
		break;
	case moderate:
		if (input <= 62.5) mu = 0.0;
		else if (input <= 80.0) mu = (input - 62.5) / (80.0 - 62.5);
		else if (input < 110.0) mu = 1.0 - (input - 80.0) / (110.0 - 80.0);
		else mu = 0.0;
		break;
	case fast:
		if (input <= 80.0) mu = 0.0;
		else if (input < 110.0) mu = (input - 80.0) / (110.0 - 80.0);
		else if (input <= 125.0) mu = 1.0;
		else if (input < 150.0) mu = 1.0 - (input - 125.0) / (150.0 - 125.0);
		else mu = 0.0;
		break;
	case vfast:
		if (input <= 125.0) mu = 0.0;
		else if (input < 150.0) mu = (input - 125.0) / (150.0 - 125.0);
		else mu = 1.0;
		break;
	default:
		Helpers::print_debug("ERROR: undefined tempo class requested.\n");
		mu = 0.0;
		break;
	}
	return mu;
}

double OptiAlgo::FLSystem::IntensInClass(double input, IntensClassIDs flClass) {
	double mu;
	switch (flClass) {
	case quiet:
		if (input <= 3000) mu = 1.0;
		else if (input < 3500) mu = 1.0 - (input - 3000) / (3500 - 3000);
		else mu = 0.0;
		break;
	case mid:
		if (input <= 3150) mu = 0.0;
		else if (input <= 4000) mu = (input - 3150) / (4000 - 3150);
		else if (input < 4800) mu = 1.0 - (input - 4000) / (4800 - 4000);
		else mu = 0.0;
		break;
	case loud:
		if (input <= 4200) mu = 0.0;
		else if (input < 5000) mu = (input - 4200) / (5000 - 4200);
		else mu = 1.0;
		break;
	default:
		Helpers::print_debug("ERROR: undefined intens class requested.\n");
		mu = 0.0;
		break;
	}
	return mu;
}

// TODO: implement beatiness' values
double OptiAlgo::FLSystem::BeatinessInClass(double input, BeatinessClassIDs flClass) {
	double mu;
	switch (flClass) {
	case notbeaty:
		if (input <= 3000) mu = 1.0;
		else if (input < 3500) mu = 1.0 - (input - 3000) / (3500 - 3000);
		else mu = 0.0;
		break;
	case sbeaty:
		if (input <= 3150) mu = 0.0;
		else if (input <= 4000) mu = (input - 3150) / (4000 - 3150);
		else if (input < 4800) mu = 1.0 - (input - 4000) / (4800 - 4000);
		else mu = 0.0;
		break;
	case beaty:
		if (input <= 3150) mu = 0.0;
		else if (input <= 4000) mu = (input - 3150) / (4000 - 3150);
		else if (input < 4800) mu = 1.0 - (input - 4000) / (4800 - 4000);
		else mu = 0.0;
		break;
	case vbeaty:
		if (input <= 4200) mu = 0.0;
		else if (input < 5000) mu = (input - 4200) / (5000 - 4200);
		else mu = 1.0;
		break;
	default:
		Helpers::print_debug("ERROR: undefined beatiness class requested.\n");
		mu = 0.0;
		break;
	}
	return mu;
}

double OptiAlgo::FLSystem::ROutClass(double input, RGBClassIDs flClass) {
	double mu;
	switch (flClass) {
	case none:
		if (input <= 64.0) mu = 1.0;
		else if (input < 128.0) mu = 1.0 - (input - 64.0) / (128.0 - 64.0);
		else mu = 0.0;
		break;
	case dark:
		if (input <= 64.0) mu = 0.0;
		else if (input <= 128.0) mu = (input - 64.0) / (128.0 - 64.0);
		else if (input < 192.0) mu = 1.0 - (input - 128.0) / (192.0 - 128.0);
		else mu = 0.0;
		break;
	case medium:
		if (input <= 128.0) mu = 0.0;
		else if (input <= 192.0) mu = (input - 128.0) / (192.0 - 128.0);
		else if (input < 255.0) mu = 1.0 - (input - 192.0) / (255.0 - 192.0);
		else mu = 0.0;
		break;
	case strong:
		if (input <= 192.0) mu = 0.0;
		else if (input < 255.0) mu = (input - 192.0) / (255.0- 192.0);
		else mu = 1.0;
		break;
	default:
		Helpers::print_debug("ERROR: undefined RB class requested.\n");
		mu = 0.0;
		break;
	}
	return (mu < Rcutoff[flClass]) ? mu : Rcutoff[flClass];
}

double OptiAlgo::FLSystem::BOutClass(double input, RGBClassIDs flClass) {
	double mu;
	switch (flClass) {
	case none:
		if (input <= 64.0) mu = 1.0;
		else if (input < 128.0) mu = 1.0 - (input - 64.0) / (128.0 - 64.0);
		else mu = 0.0;
		break;
	case dark:
		if (input <= 64.0) mu = 0.0;
		else if (input <= 128.0) mu = (input - 64.0) / (128.0 - 64.0);
		else if (input < 192.0) mu = 1.0 - (input - 128.0) / (192.0 - 128.0);
		else mu = 0.0;
		break;
	case medium:
		if (input <= 128.0) mu = 0.0;
		else if (input <= 192.0) mu = (input - 128.0) / (192.0 - 128.0);
		else if (input < 255.0) mu = 1.0 - (input - 192.0) / (255.0 - 192.0);
		else mu = 0.0;
		break;
	case strong:
		if (input <= 192.0) mu = 0.0;
		else if (input < 255.0) mu = (input - 192.0) / (255.0 - 192.0);
		else mu = 1.0;
		break;
	default:
		Helpers::print_debug("ERROR: undefined RB class requested.\n");
		mu = 0.0;
		break;
	}
	return (mu < Bcutoff[flClass]) ? mu : Bcutoff[flClass];
}

double OptiAlgo::FLSystem::GpercentOutClass(double input, RGBClassIDs flClass) {
	double mu;
	switch (flClass) {
	case none:
		if (input <= 25.0) mu = 1.0;
		else if (input < 50.0) mu = 1.0 - (input - 25.0) / (50.0 - 25.0);
		else mu = 0.0;
		break;
	case dark:
		if (input <= 25.0) mu = 0.0;
		else if (input <= 50.0) mu = (input - 25.0) / (50.0 - 25.0);
		else if (input < 75.0) mu = 1.0 - (input - 50.0) / (75.0 - 50.0);
		else mu = 0.0;
		break;
	case medium:
		if (input <= 50.0) mu = 0.0;
		else if (input <= 75.0) mu = (input - 50.0) / (75.0 - 50.0);
		else if (input < 100.0) mu = 1.0 - (input - 75.0) / (100.0 - 75.0);
		else mu = 0.0;
		break;
	case strong:
		if (input <= 75.0) mu = 0.0;
		else if (input < 100.0) mu = (input - 75.0) / (100.0 - 75.0);
		else mu = 1.0;
		break;
	default:
		Helpers::print_debug("ERROR: undefined Gpercent class requested.\n");
		mu = 0.0;
		break;
	}
	return (mu < Gcutoff[flClass]) ? mu : Gcutoff[flClass];
}

double OptiAlgo::FLSystem::WOutClass(double input, WClassIDs flClass) {
	double mu;
	switch (flClass) {
	case none:
		if (input <= 50.0) mu = 1.0;
		else if (input < 75.0) mu = 1.0 - (input - 50.0) / (75.0 - 50.0);
		else mu = 0.0;
		break;
	case WClassIDs::normal:
		if (input <= 50.0) mu = 0.0;
		else if (input <= 75.0) mu = (input - 50.0) / (75.0 - 50.0);
		else if (input < 100.0) mu = 1.0 - (input - 75.0) / (100.0 - 75.0);
		else mu = 0.0;
		break;
	case WClassIDs::bright:
		if (input <= 75.0) mu = 0.0;
		else if (input < 100.0) mu = (input - 75.0) / (100.0 - 75.0);
		else mu = 1.0;
		break;
	default:
		Helpers::print_debug("ERROR: undefined W class requested.\n");
		mu = 0.0;
		break;
	}
	return (mu < Wcutoff[flClass]) ? mu : Wcutoff[flClass];
}

#pragma endregion

double OptiAlgo::FLSystem::Tnorm(vector<double> inputs) {
	double output = 1.0;
	for each (double input in inputs) {
		if (input > 1.0 || input < 0.0) Helpers::print_debug("ERROR: OptiAlgo: Tnorm encountered out-of-bounds mu; skipping value.\n");
		else output = min(output, input);
	}
	return output;
}

template <typename T>
T OptiAlgo::FLSystem::GetHighestCutoff(map <T, double> cutoff) {
	T classID = (T)0;
	double max_val = 0.0;
	for (map<T,double>::iterator it = cutoff.begin(); it != cutoff.end(); it++) {
		if (it->second > max_val) {
			max_val = it->second;
			classID = it->first;
		}
	}
	
	return classID;
}

// TODO: make G a % by taking % of red or blue (is this a good idea though? Talk to Brandon about it)
double OptiAlgo::FLSystem::DefuzzifyRGB(RGBClassIDs classID) {
	double ret_val;
	switch (classID) {
	case none:
		ret_val = 64.0;
		break;
	case dark:
		ret_val = 128.0;
		break;
	case medium:
		ret_val = 192.0;
		break;
	case strong:
		ret_val = 255.0;;
		break;
	default:
		Helpers::print_debug("ERROR: OptiAlgo: RGB defuzzification didn't get correct class; defaulting to 0.\n");
		ret_val = 0.0;
		break;
	}
	if (ret_val < 0.0) Helpers::print_debug("ERROR: OptiAlgo: RGB defuzzification returning negative value!\n");
	return ret_val;
}

double OptiAlgo::FLSystem::DefuzzifyW(WClassIDs classID) {
	double ret_val;
	switch (classID) {
	case off:
		ret_val = 50.0;
		break;
	case normal:
		ret_val = 75.0;
		break;
	case bright:
		ret_val = 100.0;
		break;
	default:
		Helpers::print_debug("ERROR: OptiAlgo: W defuzzification didn't get correct class; defaulting to 0.\n");
		ret_val = 0.0;
		break;
	}
	if (ret_val < 0.0) Helpers::print_debug("ERROR: OptiAlgo: W defuzzification returning negative value!\n");
	return ret_val;
}


LightsInfo OptiAlgo::FLSystem::Infer(AudioProps input) {
	vector<double> tnorm_temp;
	LightsInfo out_crisp;

	// Reset cutoffs before doing anything
	ResetCutoffs();

	// Use generic inference rules for testing		// TODO: make this real
	Rcutoff[none] = FreqInClass(input.freq_avg, vlow);		// F = vlow => R = none
	Rcutoff[dark] = FreqInClass(input.freq_avg, low);		// F = low => R = dark
	Rcutoff[medium] = FreqInClass(input.freq_avg, high);	// F = high => R = medium
	Rcutoff[strong] = FreqInClass(input.freq_avg, vhigh);	// F = vhigh => R = strong

	tnorm_temp.clear();
	tnorm_temp.push_back(TempoInClass(input.tempo_avg, vslow));
	tnorm_temp.push_back(TempoInClass(input.tempo_avg, slow));
	Bcutoff[none] = Tnorm(tnorm_temp);							// T = vslow || T = slow => B = none
	Bcutoff[dark] = TempoInClass(input.tempo_avg, moderate);	// T = moderate => B = dark
	Bcutoff[medium] = TempoInClass(input.tempo_avg, fast);		// T = fast => B = medium
	Bcutoff[strong] = TempoInClass(input.tempo_avg, vfast);		// T = vfast => B = strong

	// TODO: add this after beatiness values are tweaked properly
	double beatiness = input.loudness_max_avg - input.loudness_nomax_avg;
	//Gcutoff[none] = BeatinessInClass(beatiness, notbeaty);		// B = vlow => G = none
	//Gcutoff[dark] = BeatinessInClass(beatiness, sbeaty);		// B = low => G = dark
	//Gcutoff[medium] = BeatinessInClass(beatiness, beaty);	// B = high => G = medium
	//Gcutoff[strong] = BeatinessInClass(beatiness, vbeaty);	// B = vhigh => G = strong
	Gcutoff[none] = 0.0;
	Gcutoff[dark] = 0.0;
	Gcutoff[medium] = 0.0;
	Gcutoff[strong] = 0.0;

	Wcutoff[off] = IntensInClass(input.loudness_avg, quiet);		// W is never off
	Wcutoff[normal] = IntensInClass(input.loudness_avg, mid);		// I = mid => W = normal
	Wcutoff[bright] = IntensInClass(input.loudness_avg, loud);		// I = high => W = bright

	// Defuzzify each output parameter // TODO: prevent magic numbers here and in mem. fn. functions by unifying them
	out_crisp.red_intensity = (int)DefuzzifyRGB(GetHighestCutoff<RGBClassIDs>(Rcutoff));
	out_crisp.green_intensity = (int)DefuzzifyRGB(GetHighestCutoff<RGBClassIDs>(Gcutoff));
	out_crisp.blue_intensity = (int)DefuzzifyRGB(GetHighestCutoff<RGBClassIDs>(Bcutoff));
	out_crisp.dimness = (int)DefuzzifyW(GetHighestCutoff<WClassIDs>(Wcutoff));
	out_crisp.strobing_speed = (beatiness > BEAT_THRESH * 3) ? (int)input.tempo_avg : 0; // TODO: tweak this - this obviously won't work

	// Done!
	return out_crisp;
}

#pragma endregion

#pragma region OptiAlgo

static queue<AudioInfo> audio_buffer = queue<AudioInfo>();

OptiAlgo::OptiAlgo()
{
	srand(static_cast<unsigned int>(time(NULL)));
	audio_buffer = queue<AudioInfo>();
	terminate = false;
}

bool OptiAlgo::receive_audio_input_sample(AudioInfo audio_sample)
{
	if (audio_buffer.size() < AUDIO_BUF_SIZE)
		audio_buffer.push(audio_sample);
	return audio_buffer.size() < AUDIO_BUF_SIZE;
}

void OptiAlgo::test_lights()
{
	LightsInfo lights_config;
	int strobe = 0;
	int w = 0;
	char * err_str;

	while (!terminate)
	{
		try
		{
			lights_config.red_intensity = 255;
			lights_config.blue_intensity = 0;
			lights_config.green_intensity = 0;
			lights_config.white_intensity = w;
			lights_config.strobing_speed = strobe;
			lights_config.dimness = 255;
			DMXOutput::updateLightsOutputQueue(lights_config);
			Sleep(200);
			lights_config.red_intensity = 0;
			lights_config.blue_intensity = 0;
			lights_config.green_intensity = 255;
			lights_config.white_intensity = w;
			lights_config.strobing_speed = strobe;
			lights_config.dimness = 255;
			DMXOutput::updateLightsOutputQueue(lights_config);
			Sleep(200);
			lights_config.red_intensity = 0;
			lights_config.blue_intensity = 255;
			lights_config.green_intensity = 0;
			lights_config.white_intensity = w;
			lights_config.strobing_speed = strobe;
			lights_config.dimness = 255;
			DMXOutput::updateLightsOutputQueue(lights_config);
			//strobe = (strobe + 100) % 200;
			w += 10;
			Sleep(200);
		}
		catch (exception e)
		{
			err_str = "";
			strcat(err_str, "ERROR: OptiAlgo: ");
			strcat(err_str, e.what());
			strcat(err_str, "\n");
			Helpers::print_debug(err_str);
		}
	}
	Helpers::print_debug("OptiAlgo: stopped.\n");
}

void OptiAlgo::start_algo()
{
	// Initialize input variables
	AudioInfo audio_sample, smoothed_input;
	AudioProps input = AudioProps();
	double cur_freq, cur_tempo, cur_loud;
	bool got_freq, got_tempo, got_loud;
	// Initialize solution variables
	FLSystem flSystem = FLSystem();
	LightsInfo cur_sol;
	// Initialize output variables
	deque<LightsInfo> out_hist = deque<LightsInfo>();
	LightsInfo avg_out = LightsInfo(true);
	// Initialize print-loggers
	stringstream out_str;
	char * err_str;


	// Start main execution loop
	while (!terminate)
	{
		try {
			// Wait for audio input samples
			while (audio_buffer.empty()) {
				if (terminate) { return; }
			}

			out_str.str("");

			// Grab sample
			audio_sample = audio_buffer.front();
			got_freq = audio_sample.get_frequency(cur_freq);
			got_tempo = audio_sample.get_tempo(cur_tempo);
			got_loud = audio_sample.get_loudness(cur_loud);
			audio_buffer.pop();

			// Are we still silent?
			if (got_loud) input.silence = (cur_loud < SILENCE_THRESH);

			// Update averages & history
			if (!input.silence)
			{
				if (got_freq) input.freq_add_and_avg(cur_freq);

				if (got_tempo) input.tempo_add_and_avg(cur_tempo);

				if (got_loud)
				{
					input.loudness_hist_add_and_avg(cur_loud);
					if (input.loudness_avg > 0.0 && cur_loud - input.loudness_avg >= BEAT_THRESH)
						input.loudness_max_hist_add_and_avg(cur_loud);
					else
						input.loudness_nomax_hist_add_and_avg(cur_loud);

					// Uncomment this if necessary
					/*if (abs(input.loudness_avg - input.loudness_max_avg) >= CHANGE_TO_MAX_LOUD_THRESH)
					{
						input.loudness_nomax_avg = input.loudness_avg;
						input.loudness_nomax_hist = input.loudness_hist;
						input.loudness_hist.clear();
					}*/
				}
			}

			// Log smoothed input
			out_str << "[OA] S=" << input.silence << " ";
			out_str << "in_avg:[F, T, L(nm, m)] = ["
				<< input.freq_avg << "," << input.tempo_avg << ","
				<< input.loudness_avg << "(" << input.loudness_nomax_avg << "," << input.loudness_max_avg << ")]";

			// Fuzzy inference
			if (!input.silence) {
				// Use fuzzy logic system to get output
				cur_sol = flSystem.Infer(input);

				// Average output
				out_hist.push_front(cur_sol);
				if (out_hist.size() > OUT_HIST_BUF_SIZE) out_hist.pop_back();
				avg_out = LightsInfo::average_and_smooth(out_hist);
			}
			else {
				// On silence, clear output history and output 0's to lights
				out_hist.clear();
				cur_sol = LightsInfo();
				avg_out = cur_sol;
			}

			// Log current output
			out_str << " => ";
			out_str << "out:[R,G,B,W,Dim,Strobe]=[" << cur_sol.red_intensity << "," << cur_sol.green_intensity << ","
				<< cur_sol.blue_intensity << "," << cur_sol.white_intensity << "," << cur_sol.dimness << ","
				<< cur_sol.strobing_speed << "]";

			// Log smoothed output
			out_str << " => ";
			out_str << "out_avg:[R,G,B,W,Dim,Strobe]=[" << avg_out.red_intensity << "," << avg_out.green_intensity << ","
				<< avg_out.blue_intensity << "," << avg_out.white_intensity << "," << avg_out.dimness << ","
				<< avg_out.strobing_speed << "]";

			// Print log to output window!
			out_str << "\n";
			//Helpers::print_debug(out_str.str().c_str());

			// Send solution to output controller
			DMXOutput::updateLightsOutputQueue(avg_out);

		}
		catch (exception e)
		{	
			err_str = "";
			strcat(err_str, "ERROR: OptiAlgo: ");
			strcat(err_str, e.what());
			strcat(err_str, "\n");
			Helpers::print_debug(err_str);
		}
	}
	if (terminate) Helpers::print_debug("OptiAlgo: terminated.\n");
	else Helpers::print_debug("OptiAlgo: stopped.\n");
}


void OptiAlgo::start()
{
	start_algo();
	//test_lights();
}

void OptiAlgo::stop()
{
	terminate = true;
}

#pragma endregion