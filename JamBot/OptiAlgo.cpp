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
#include <cstdlib>

#include "Constructs.h"
#include "Constants.h"
#include "Helpers.h"
#include "OptiAlgo.h"
#include "DMXOutput.h"


using namespace std;

// Output logger
stringstream out_str;


#pragma region AudioProps

OptiAlgo::AudioProps::AudioProps()
{
	freq_avg = 0.0;
	delta_freq_avg = 0.0;
	tempo_avg = 0.0;
	loudness_avg = 0.0;
	loudness_nomax_avg = 0.0;
	loudness_max_avg = 0.0;
}

void OptiAlgo::AudioProps::ClearProps()
{
	freq_avg = 0.0;
	delta_freq_avg = 0.0;
	tempo_avg = 0.0;
	loudness_avg = 0.0;
	loudness_nomax_avg = 0.0;
	loudness_max_avg = 0.0;

	freq_hist.clear();
	delta_freq_hist.clear();
	tempo_hist.clear();
	loudness_hist.clear();
	loudness_max_hist.clear();
	loudness_nomax_hist.clear();
}

double OptiAlgo::AudioProps::freq_add_and_avg(double val)
{
	double avg = 0.0;
	freq_hist.push_front(val);
	if (freq_hist.size() > IN_HIST_BUF_SIZE) freq_hist.pop_back();
	for each (double val in freq_hist)
		avg += val;
	avg /= (double)freq_hist.size();
	freq_avg = avg;
	return avg;
}
double OptiAlgo::AudioProps::delta_freq_add_and_avg(double val)
{
	double avg = 0.0;
	delta_freq_hist.push_front(val);
	if (delta_freq_hist.size() > IN_HIST_BUF_SIZE) delta_freq_hist.pop_back();
	for each (double val in delta_freq_hist)
		avg += val;
	avg /= (double)delta_freq_hist.size();
	delta_freq_avg = avg;
	return avg;
}
double OptiAlgo::AudioProps::tempo_add_and_avg(double val)
{
	double avg = 0.0;
	tempo_hist.push_front(val);
	if (tempo_hist.size() > IN_HIST_BUF_SIZE) tempo_hist.pop_back();
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
	if (loudness_hist.size() > IN_HIST_BUF_SIZE) loudness_hist.pop_back();
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
	if (loudness_nomax_hist.size() > IN_HIST_BUF_SIZE) loudness_nomax_hist.pop_back();
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
	if (loudness_max_hist.size() > IN_HIST_BUF_SIZE) loudness_max_hist.pop_back();
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
		if (input <= 100.0) mu = 1.0;
		else if (input < 150.0) mu = 1.0 - (input - 100.0) / (150.0 - 100.0);
		else mu = 0.0;
		break;
	case low:
		if (input <= 100.0) mu = 0.0;
		else if (input <= 150.0) mu = (input - 100.0) / (150.0 - 100.0);
		else if (input <= 200.0) mu = 1.0;
		else if (input < 250.0) mu = 1.0 - (input - 200.0) / (250.0 - 200.0);
		else mu = 0.0;
		break;
	case high:
		if (input <= 200.0) mu = 0.0;
		else if (input <= 250.0) mu = (input - 200.0) / (250.0 - 200.0);
		else if (input <= 300.0) mu = 1.0;
		else if (input < 350.0) mu = 1.0 - (input - 300.0) / (350.0 - 300.0);
		else mu = 0.0;
		break;
	case vhigh:
		if (input <= 300.0) mu = 0.0;
		else if (input < 350.0) mu = (input - 300.0) / (350.0 - 300.0);
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
		if (input <= 200.0) mu = 1.0;
		else if (input < 400.0) mu = 1.0 - (input - 400.0) / (200.0 - 400.0);
		else mu = 0.0;
		break;
	case mid:
		if (input <= 200.0) mu = 0.0;
		else if (input <= 400.0) mu = (input - 200.0) / (400.0 - 200.0);
		else if (input <= 800.0) mu = 1.0;
		else if (input < 1200.0) mu = 1.0 - (input - 800.0) / (1200.0 - 800.0);
		else mu = 0.0;
		break;
	case loud:
		if (input <= 800.0) mu = 0.0;
		else if (input < 1200.0) mu = (input - 800.0) / (1200.0 - 800.0);
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
	double none[] = { 0.0-85.0, 0.0, 85.0 };
	double dark[] = { 0.0, 85.0, 170.0 };
	double medium[] = { 85.0, 170.0, 255.0 };
	double strong[] = { 170.0, 255.0, 255.0+85.0 };
	double mu;

	double * points;
	switch (flClass) {
	case RGBClassIDs::none:
		points = none;
		break;
	case RGBClassIDs::dark:
		points = dark;
		break;
	case RGBClassIDs::medium:
		points = medium;
		break;
	case RGBClassIDs::strong:
		points = strong;
		break;
	default:
		Helpers::print_debug("ERROR: undefined R class requested.\n");
		points = new double [] { 0.0, 0.0, 0.0 };
		break;
	}

	if (input <= points[0]) mu = 0.0;
	else if (input <= points[1]) mu = (input - points[0]) / (points[1] - points[0]);
	else if (input < points[2]) mu = 1.0 - (input - points[1]) / (points[2] - points[1]);
	else mu = 0.0;

	return (mu < Rcutoff[flClass]) ? mu : Rcutoff[flClass];
}

double OptiAlgo::FLSystem::BOutClass(double input, RGBClassIDs flClass) {
	double none[] = { 0.0 - 85.0, 0.0, 85.0 };
	double dark[] = { 0.0, 85.0, 170.0 };
	double medium[] = { 85.0, 170.0, 255.0 };
	double strong[] = { 170.0, 255.0, 255.0 + 85.0 };
	double mu;

	double * points;
	switch (flClass) {
	case RGBClassIDs::none:
		points = none;
		break;
	case RGBClassIDs::dark:
		points = dark;
		break;
	case RGBClassIDs::medium:
		points = medium;
		break;
	case RGBClassIDs::strong:
		points = strong;
		break;
	default:
		Helpers::print_debug("ERROR: undefined B class requested.\n");
		points = new double [] { 0.0, 0.0, 0.0 };
		break;
	}

	if (input <= points[0]) mu = 0.0;
	else if (input <= points[1]) mu = (input - points[0]) / (points[1] - points[0]);
	else if (input < points[2]) mu = 1.0 - (input - points[1]) / (points[2] - points[1]);
	else mu = 0.0;

	return (mu < Bcutoff[flClass]) ? mu : Bcutoff[flClass];
}

double OptiAlgo::FLSystem::GOutClass(double input, RGBClassIDs flClass) {
	double mu;
	switch (flClass) {
	case none:
		if (input <= 0.0) mu = 1.0;
		else if (input < 64.0) mu = 1.0 - (input - 0.0) / (64.0 - 0.0);
		else mu = 0.0;
		break;
	case dark:
		if (input <= 0.0) mu = 0.0;
		else if (input <= 64.0) mu = (input - 0.0) / (64.0 - 0.0);
		else if (input < 128.0) mu = 1.0 - (input - 64.0) / (128.0 - 64.0);
		else mu = 0.0;
		break;
	case medium:
		if (input <= 64.0) mu = 0.0;
		else if (input <= 128.0) mu = (input - 64.0) / (128.0 - 64.0);
		else if (input < 192.0) mu = 1.0 - (input - 128.0) / (192.0 - 128.0);
		else mu = 0.0;
		break;
	case strong:
		if (input <= 128.0) mu = 0.0;
		else if (input < 192.0) mu = (input - 128.0) / (192.0 - 128.0);
		else mu = 1.0;
		break;
	default:
		Helpers::print_debug("ERROR: undefined G class requested.\n");
		mu = 0.0;
		break;
	}
	return (mu < Gcutoff[flClass]) ? mu : Gcutoff[flClass];
}

//double OptiAlgo::FLSystem::WOutClass(double input, WClassIDs flClass) {
//	double mu;
//	switch (flClass) {
//	case none:
//		if (input <= 50.0) mu = 1.0;
//		else if (input < 75.0) mu = 1.0 - (input - 50.0) / (75.0 - 50.0);
//		else mu = 0.0;
//		break;
//	case WClassIDs::normal:
//		if (input <= 50.0) mu = 0.0;
//		else if (input <= 75.0) mu = (input - 50.0) / (75.0 - 50.0);
//		else if (input < 100.0) mu = 1.0 - (input - 75.0) / (100.0 - 75.0);
//		else mu = 0.0;
//		break;
//	case WClassIDs::bright:
//		if (input <= 75.0) mu = 0.0;
//		else if (input < 100.0) mu = (input - 75.0) / (100.0 - 75.0);
//		else mu = 1.0;
//		break;
//	default:
//		Helpers::print_debug("ERROR: undefined W class requested.\n");
//		mu = 0.0;
//		break;
//	}
//	return (mu < Wcutoff[flClass]) ? mu : Wcutoff[flClass];
//}

#pragma endregion

double OptiAlgo::FLSystem::Tnorm(vector<double> inputs) {
	double output = 1.0;
	for each (double input in inputs) {
		if (input > 1.0 || input < 0.0) Helpers::print_debug("ERROR: OptiAlgo: Tnorm encountered out-of-bounds mu; skipping value.\n");
		else output = min(output, input);
	}
	return output;
}

double OptiAlgo::FLSystem::Integrate(OutParams outparam, double lb, double ub, double step = 1.0) {
	double area_accumulator = 0.0;

	switch (outparam) {
	case r:
		OutClass = &OptiAlgo::FLSystem::ROutClass;
		break;
	case b:
		OutClass = &OptiAlgo::FLSystem::BOutClass;
		break;
	case g:
		OutClass = &OptiAlgo::FLSystem::GOutClass;
		break;
	default:
		Helpers::print_debug("ERROR: undefined parameter requested in integration; returning 0.\n");
		return 0.0;
	}

	double h = 0.0;
	for (double x = lb; x <= ub - step; x += step) {
		h = max(max(max((this->*OutClass)(x, none), (this->*OutClass)(x, dark)),
						(this->*OutClass)(x, medium)), (this->*OutClass)(x, strong));

		area_accumulator += h * step;
	}
	return area_accumulator;
}

double OptiAlgo::FLSystem::Defuzzify(OutParams outparam) {
	double lb = 0.0 - 85.0; // Account for the negative sides of outlying classes
	double ub = 255.0 + 85.0; // Account for 'overshooting' sides of outlying classes
	double target_area = Integrate(outparam, lb, ub) / 2;

	if (target_area < 0.0 || target_area > 255.0) {
		Helpers::print_debug("ERROR: defuzzification target area out of range; returning 0.\n");
		return 0.0;
	}

	const double step = 1.0;
	double area_accumulator = 0.0;
	double step_lb = lb;
	double step_ub = step_lb + step;

	while (area_accumulator < target_area) {
		area_accumulator += Integrate(outparam, step_lb, step_ub);
		step_lb += step;
		step_ub += step;
	}

	double ret_val = (step_lb > OUTPUT_TOO_LOW_THRESH) ? step_lb : 0.0; // Area slightly overshoots so compensate by sending lower bound
	if (ret_val < 0.0) ret_val = 0.0;
	else if (ret_val > 255.0) ret_val = 255.0;
	return ret_val;
}

LightsInfo OptiAlgo::FLSystem::Infer(AudioProps input) {
	double beatiness = input.loudness_max_avg - input.loudness_nomax_avg;
	double strobe_speed;
	LightsInfo out_crisp;
	vector<double> tnorm_temp;

	// Reset cutoffs before doing anything
	ResetCutoffs();

	// Use generic inference rules for testing
	// TODO: for intensity: should I use loudness_avg or loudness_nomax_avg?
	// RED
	tnorm_temp.clear();
	tnorm_temp.push_back(FreqInClass(input.freq_avg, vlow));
	tnorm_temp.push_back(IntensInClass(input.loudness_avg, loud));
	Rcutoff[strong] = Tnorm(tnorm_temp);

	tnorm_temp.clear();
	tnorm_temp.push_back(FreqInClass(input.freq_avg, low));
	tnorm_temp.push_back(IntensInClass(input.loudness_avg, loud));
	Rcutoff[medium] = Tnorm(tnorm_temp);

	tnorm_temp.clear();
	tnorm_temp.push_back(max(FreqInClass(input.freq_avg, vlow), FreqInClass(input.freq_avg, low)));
	tnorm_temp.push_back(IntensInClass(input.loudness_avg, mid));
	Rcutoff[dark] = Tnorm(tnorm_temp);

	tnorm_temp.clear();
	tnorm_temp.push_back(max(max(FreqInClass(input.freq_avg, high), FreqInClass(input.freq_avg, vhigh)),
								 IntensInClass(input.loudness_avg, quiet)) );
	Rcutoff[none] = Tnorm(tnorm_temp);

	out_str << "Rcutoff: ";
	for (int i = 0; i < 4; i++) {
		out_str << to_string((RGBClassIDs)i) << ": " << Rcutoff[(RGBClassIDs)i] << "; ";
	}
	out_str << "\n";


	// BLUE
	tnorm_temp.clear();
	tnorm_temp.push_back(FreqInClass(input.freq_avg, vhigh));
	tnorm_temp.push_back(IntensInClass(input.loudness_avg, loud));
	Bcutoff[strong] = Tnorm(tnorm_temp);

	tnorm_temp.clear();
	tnorm_temp.push_back(FreqInClass(input.freq_avg, high));
	tnorm_temp.push_back(IntensInClass(input.loudness_avg, loud));
	Bcutoff[medium] = Tnorm(tnorm_temp);

	tnorm_temp.clear();
	tnorm_temp.push_back(max(FreqInClass(input.freq_avg, high), FreqInClass(input.freq_avg, vhigh)));
	tnorm_temp.push_back(IntensInClass(input.loudness_avg, mid));
	Bcutoff[dark] = Tnorm(tnorm_temp);

	tnorm_temp.clear();
	tnorm_temp.push_back(max(max(FreqInClass(input.freq_avg, vlow), FreqInClass(input.freq_avg, low)),
								 IntensInClass(input.loudness_avg, quiet)));
	Bcutoff[none] = Tnorm(tnorm_temp);

	out_str << "Bcutoff: ";
	for (int i = 0; i < 4; i++) {
		out_str << to_string((RGBClassIDs)i) << ": " << Bcutoff[(RGBClassIDs)i] << "; ";
	}
	out_str << "\n";


	//// GREEN
	//tnorm_temp.clear();
	//tnorm_temp.push_back(FreqInClass(input.freq_avg, vhigh));
	//tnorm_temp.push_back(TempoInClass(input.tempo_avg, vfast));
	//tnorm_temp.push_back(IntensInClass(input.loudness_avg, loud));
	//Gcutoff[strong] = Tnorm(tnorm_temp);

	//tnorm_temp.clear();
	//tnorm_temp.push_back(FreqInClass(input.freq_avg, high));
	//tnorm_temp.push_back(TempoInClass(input.tempo_avg, fast));
	//tnorm_temp.push_back(IntensInClass(input.loudness_avg, loud));
	//Gcutoff[medium] = Tnorm(tnorm_temp);

	//tnorm_temp.clear();
	//tnorm_temp.push_back(max(FreqInClass(input.freq_avg, high), FreqInClass(input.freq_avg, vhigh)));
	//tnorm_temp.push_back(max(TempoInClass(input.tempo_avg, fast), TempoInClass(input.tempo_avg, vfast)));
	//tnorm_temp.push_back(IntensInClass(input.loudness_avg, mid));
	//temp = Tnorm(tnorm_temp);

	//tnorm_temp.clear();
	//tnorm_temp.push_back(max(FreqInClass(input.freq_avg, low), FreqInClass(input.freq_avg, vlow)));
	//tnorm_temp.push_back(max(TempoInClass(input.tempo_avg, fast), TempoInClass(input.tempo_avg, vfast)));
	//tnorm_temp.push_back(IntensInClass(input.loudness_avg, mid));
	//Gcutoff[dark] = max(temp, Tnorm(tnorm_temp));

	//tnorm_temp.clear();
	//tnorm_temp.push_back(max(max(max(TempoInClass(input.freq_avg, vslow), TempoInClass(input.freq_avg, slow)), TempoInClass(input.freq_avg, moderate)),
	//	IntensInClass(input.loudness_avg, quiet)));
	//Gcutoff[none] = Tnorm(tnorm_temp);

	//out_str << "Gcutoff: ";
	//for (int i = 0; i < 4; i++) {
	//	out_str << to_string((RGBClassIDs)i) << ": " << Gcutoff[(RGBClassIDs)i] << "; ";
	//}
	//out_str << "\n";


	// Defuzzify each output parameter
	out_crisp.red_intensity = (int)Defuzzify(r);
	out_crisp.blue_intensity = (int)Defuzzify(b);
	out_crisp.green_intensity = 0; //(int)DefuzzifyRGB(GetHighestCutoff<RGBClassIDs>(Gcutoff)); // hack
	out_crisp.white_intensity = 0;
	out_crisp.dimness = 255;

	// For strobing, just using speed as tempo_avg causes a lag of a 16th note (quarter of a beat)
	// (instead of falling on the next 'ta', it falls on the 'ka' after that, in the ta-ka-di-mi system)
	// Dividing by 5 gives you one 16th note; multiplying by 4 sets it for one quarter note (one beat)
	strobe_speed = 4 * input.tempo_avg / 5;
	strobe_speed *= 2; // Double it to make strobing look nice
	out_crisp.strobing_speed = 0;//(beatiness > BEATINESS_THRESH) ? (int)strobe_speed : 0;

	// Done!
	return out_crisp;
}

#pragma endregion

#pragma region OptiAlgo

static queue<AudioInfo> audio_buffer = queue<AudioInfo>();
static queue<SECTION> song_section_buffer = queue<SECTION>();

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

bool OptiAlgo::receive_song_section(SECTION audio_sample)
{
	if (song_section_buffer.size() < AUDIO_BUF_SIZE)
		song_section_buffer.push(audio_sample);
	return song_section_buffer.size() < AUDIO_BUF_SIZE;
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
			Sleep(2000);
			lights_config.red_intensity = 0;
			lights_config.blue_intensity = 0;
			lights_config.green_intensity = 255;
			lights_config.white_intensity = w;
			lights_config.strobing_speed = strobe;
			lights_config.dimness = 255;
			DMXOutput::updateLightsOutputQueue(lights_config);
			Sleep(2000);
			lights_config.red_intensity = 0;
			lights_config.blue_intensity = 255;
			lights_config.green_intensity = 0;
			lights_config.white_intensity = w;
			lights_config.strobing_speed = strobe;
			lights_config.dimness = 255;
			DMXOutput::updateLightsOutputQueue(lights_config);
			w += 255;
			Sleep(2000);
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
	int nudges_to_silence = NUDGES_TO_SILENCE;
	bool be_quiet = false;
	// Initialize solution variables
	FLSystem flSystem = FLSystem();
	LightsInfo cur_sol;
	// Initialize output variables
	deque<LightsInfo> out_hist = deque<LightsInfo>();
	deque<LightsInfo> delta_out_hist = deque<LightsInfo>();
	LightsInfo avg_out = LightsInfo(true);
	LightsInfo old_avg_out;
	// Initialize print-loggers
	char * err_str;

	// Start main execution loop
	while (!terminate)
	{
		try {
			// Wait for audio input samples
			while (audio_buffer.empty()) {
				if (terminate) { return; }
			}

			// Reset logging stream
			out_str.str("");

			// Grab sample
			audio_sample = audio_buffer.front();
			got_freq = audio_sample.get_frequency(cur_freq);
			got_tempo = audio_sample.get_tempo(cur_tempo);
			got_loud = audio_sample.get_loudness(cur_loud);
			audio_buffer.pop();

			// Flush buffers if section changes
			if (!song_section_buffer.empty()) {
				Helpers::print_debug("[FL] Section change!\n");
				input.freq_hist.clear();
				input.tempo_hist.clear();
				input.loudness_hist.clear();
				input.loudness_max_hist.clear();
				input.loudness_nomax_hist.clear();
			}

			// Are we still silent?
			if (got_loud) {
				if (cur_loud < SILENCE_THRESH) {
					input.silence = (nudges_to_silence <= 0);
					nudges_to_silence--;
				}
				else {
					input.silence = false;
					nudges_to_silence = NUDGES_TO_SILENCE;
				}
			}

			// Update averages & history
			if (!input.silence)
			{
				if (got_freq) {
					// Check for harmonic frequency anomalies and correct them before inserting
					if (input.freq_hist.size() >= IN_HIST_BUF_SIZE && (abs(cur_freq - input.freq_avg * 2) < FREQ_HARMONIC_DETECT_THRESH || abs(cur_freq - input.freq_avg) > FREQ_ANOMALY_DETECT_THRESH)) {
						input.delta_freq_add_and_avg(cur_freq);
						if (input.delta_freq_hist.size() >= FREQ_NUDGES_TO_CHANGE) {
							input.freq_hist = input.delta_freq_hist;
							input.freq_avg = input.delta_freq_avg;
							input.delta_freq_hist.clear();
						}
					}
					else {
						input.freq_add_and_avg(cur_freq);
						input.delta_freq_hist.clear();
					}
				}

				if (got_tempo) {
					if ((input.tempo_avg != 0.0 || (input.tempo_avg >= TEMPO_LB && input.tempo_avg <= TEMPO_UB)) &&
						(abs(cur_tempo / 2 - input.tempo_avg) <= TEMPO_CLOSENESS_THRESH) || (abs(cur_tempo * 2 - input.tempo_avg) <= TEMPO_CLOSENESS_THRESH)) {
						cur_tempo /= 2; // TODO: don't just halve it
					}
					input.tempo_add_and_avg(cur_tempo);
				}

				if (got_loud)
				{
					input.loudness_hist_add_and_avg(cur_loud);
					if (input.loudness_avg > 0.0 && cur_loud >= input.loudness_max_avg)
						input.loudness_max_hist_add_and_avg(cur_loud); // TODO: should not be persistent
					else
						input.loudness_nomax_hist_add_and_avg(cur_loud);

					// Uncomment this if necessary // TODO: make loudness_max not persistent
					/*if (abs(input.loudness_avg - input.loudness_max_avg) >= CHANGE_TO_MAX_LOUD_THRESH)
					{
						input.loudness_nomax_avg = input.loudness_avg;
						input.loudness_nomax_hist = input.loudness_hist;
						input.loudness_hist.clear();
					}*/
				}
			}

			// Fuzzy inference
			if (!input.silence) {
				// Use fuzzy logic system to get output
				cur_sol = flSystem.Infer(input);

				// Average output
				if (out_hist.size() >= OUT_HIST_BUF_SIZE && (
						abs(cur_sol.red_intensity - old_avg_out.red_intensity) > OUTPUT_ANOMALY_DETECT_THRESH ||
						abs(cur_sol.blue_intensity - old_avg_out.blue_intensity) > OUTPUT_ANOMALY_DETECT_THRESH ||
						abs(cur_sol.green_intensity - old_avg_out.green_intensity) > OUTPUT_ANOMALY_DETECT_THRESH)) {
					delta_out_hist.push_front(cur_sol);
					if (delta_out_hist.size() >= OUTPUT_NUDGES_TO_CHANGE) {
						delta_out_hist.pop_front(); // TODO: pop_back and push_front that into out_hist? 
						out_hist.push_front(cur_sol);
						if (out_hist.size() > OUT_HIST_BUF_SIZE) out_hist.pop_back();
					}
				}
				else {
					out_hist.push_front(cur_sol);
					if (out_hist.size() > OUT_HIST_BUF_SIZE) out_hist.pop_back();
					if (delta_out_hist.size() > 0) delta_out_hist.pop_back();
				}
				avg_out = LightsInfo::average_and_smooth(out_hist);
				old_avg_out = avg_out;
			}
			else {
				// On silence, clear history and output 0's to lights
				input.ClearProps();
				out_hist.clear();
				cur_sol = LightsInfo();
				avg_out = cur_sol;
			}

			// Log smoothed input
			out_str << "[FL] S=" << input.silence << " ";
			out_str << "in_avg:[F, T, L(nm, m)]=["
				<< input.freq_avg << ", " << input.tempo_avg << ", "
				<< input.loudness_avg << " (" << input.loudness_nomax_avg << ", " << input.loudness_max_avg << ")]";

			// Log current output
			out_str << " => ";
			out_str << "out:[R,G,B,W,Dim,Strobe]=[" << cur_sol.red_intensity << ", " << cur_sol.green_intensity << ", "
				<< cur_sol.blue_intensity << ", " << cur_sol.white_intensity << ", " << cur_sol.dimness << ", "
				<< cur_sol.strobing_speed << "]";

			// Log smoothed output
			out_str << " => ";
			out_str << "out_avg:[R,G,B,W,Dim,Strobe]=[" << avg_out.red_intensity << ", " << avg_out.green_intensity << ", "
				<< avg_out.blue_intensity << ", " << avg_out.white_intensity << ", " << avg_out.dimness << ", "
				<< avg_out.strobing_speed << "]";

			// Print log to output window!
			out_str << "\n";
			Helpers::print_debug(out_str.str().c_str());

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


void OptiAlgo::start(bool songSelected)
{
	start_algo();
	//test_lights();
}

void OptiAlgo::stop()
{
	terminate = true;
}

#pragma endregion