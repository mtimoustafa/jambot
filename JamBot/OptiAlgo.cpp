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
	delta_harmonic_freq_avg = 0.0;
	delta_anomaly_freq_avg = 0.0;
	tempo_avg = 0.0;
	loudness_avg = 0.0;
	loudness_nomax_avg = 0.0;
	loudness_max_avg = 0.0;
	beatiness_avg = 0.0;
}

void OptiAlgo::AudioProps::ClearProps()
{
	freq_avg = 0.0;
	delta_harmonic_freq_avg = 0.0;
	delta_anomaly_freq_avg = 0.0;
	tempo_avg = 0.0;
	delta_tempo_avg = 0.0;
	loudness_avg = 0.0;
	loudness_nomax_avg = 0.0;
	loudness_max_avg = 0.0;
	beatiness_avg = 0.0;

	freq_hist.clear();
	delta_harmonic_freq_hist.clear();
	delta_anomaly_freq_hist.clear();
	tempo_hist.clear();
	delta_tempo_hist.clear();
	loudness_hist.clear();
	loudness_nomax_hist.clear();
	loudness_max_hist.clear();
	beatiness_hist.clear();
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
double OptiAlgo::AudioProps::delta_harmonic_freq_add_and_avg(double val)
{
	double avg = 0.0;
	delta_harmonic_freq_hist.push_front(val);
	if (delta_harmonic_freq_hist.size() > IN_HIST_BUF_SIZE) delta_harmonic_freq_hist.pop_back();
	for each (double val in delta_harmonic_freq_hist)
		avg += val;
	avg /= (double)delta_harmonic_freq_hist.size();
	delta_harmonic_freq_avg = avg;
	return avg;
}
double OptiAlgo::AudioProps::delta_anomaly_freq_add_and_avg(double val)
{
	double avg = 0.0;
	delta_anomaly_freq_hist.push_front(val);
	if (delta_anomaly_freq_hist.size() > IN_HIST_BUF_SIZE) delta_anomaly_freq_hist.pop_back();
	for each (double val in delta_anomaly_freq_hist)
		avg += val;
	avg /= (double)delta_anomaly_freq_hist.size();
	delta_anomaly_freq_avg = avg;
	return avg;
}
double OptiAlgo::AudioProps::tempo_add_and_avg(double val)
{
	double avg = 0.0;
	tempo_hist.push_front(val);
	if (tempo_hist.size() > 1) tempo_hist.pop_back();
	for each (double val in tempo_hist)
		avg += val;
	avg /= (double)tempo_hist.size();
	tempo_avg = avg;
	return avg;
}
double OptiAlgo::AudioProps::delta_tempo_add_and_avg(double val)
{
	double avg = 0.0;
	delta_tempo_hist.push_front(val);
	if (delta_tempo_hist.size() > IN_HIST_BUF_SIZE) delta_tempo_hist.pop_back();
	for each (double val in delta_tempo_hist)
		avg += val;
	avg /= (double)delta_tempo_hist.size();
	delta_tempo_avg = avg;
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
	if (loudness_nomax_hist.size() > LOUDNESS_MAX_DETECT_BUF_SIZE) loudness_nomax_hist.pop_back();
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
	if (loudness_max_hist.size() > LOUDNESS_MAX_DETECT_BUF_SIZE) loudness_max_hist.pop_back();
	for each (double val in loudness_max_hist)
		avg += val;
	avg /= (double)loudness_max_hist.size();
	loudness_max_avg = avg;
	return avg;
}
double OptiAlgo::AudioProps::beatiness_hist_add_and_avg(double val)
{
	double avg = 0.0;
	if (val < 0) val = 0;
	beatiness_hist.push_front(val);
	if (beatiness_hist.size() > IN_HIST_BUF_SIZE) beatiness_hist.pop_back();
	for each (double val in beatiness_hist)
		avg += val;
	avg /= (double)beatiness_hist.size();
	beatiness_avg = avg;
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
		else if (input < 225.0) mu = 1.0 - (input - 200.0) / (225.0 - 200.0);
		else mu = 0.0;
		break;
	case high:
		if (input <= 200.0) mu = 0.0;
		else if (input <= 225.0) mu = (input - 200.0) / (225.0 - 200.0);
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
		if (input <= 80.0) mu = 1.0;
		else if (input < 90.0) mu = 1.0 - (input - 80.0) / (90.0 - 80.0);
		else mu = 0.0;
		break;
	case slow:
		if (input <= 80.0) mu = 0.0;
		else if (input <= 90.0) mu = (input - 80.0) / (90.0 - 80.0);
		else if (input <= 100.0) mu = 1.0;
		else if (input < 110.0) mu = 1.0 - (input - 100.0) / (110.0 - 100.0);
		else mu = 0.0;
		break;
	case moderate:
		if (input <= 100.0) mu = 0.0;
		else if (input <= 110.0) mu = (input - 100.0) / (110.0 - 100.0);
		else if (input <= 120.0) mu = 1.0;
		else if (input < 130.0) mu = 1.0 - (input - 120.0) / (130.0 - 120.0);
		else mu = 0.0;
		break;
	case fast:
		if (input <= 120.0) mu = 0.0;
		else if (input < 130.0) mu = (input - 120.0) / (130.0 - 120.0);
		else if (input <= 140.0) mu = 1.0;
		else if (input < 150.0) mu = 1.0 - (input - 140.0) / (150.0 - 140.0);
		else mu = 0.0;
		break;
	case vfast:
		if (input <= 140.0) mu = 0.0;
		else if (input < 150.0) mu = (input - 140.0) / (150.0 - 140.0);
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
	double none[] = { 0.0 - 42.5, 0.0, 42.5 };
	double dark[] = { 0.0, 42.5, 85.0 };
	double medium[] = { 42.5, 85.0, 127.5 };
	double strong[] = { 85.0, 127.5, 127.5 + 42.5 };
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
		Helpers::print_debug("ERROR: undefined G class requested.\n");
		points = new double [] { 0.0, 0.0, 0.0 };
		break;
	}

	if (input <= points[0]) mu = 0.0;
	else if (input <= points[1]) mu = (input - points[0]) / (points[1] - points[0]);
	else if (input < points[2]) mu = 1.0 - (input - points[1]) / (points[2] - points[1]);
	else mu = 0.0;

	return (mu < Gcutoff[flClass]) ? mu : Gcutoff[flClass];
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

map<OptiAlgo::FLSystem::RGBClassIDs, double> OptiAlgo::FLSystem::SetCutoff(OutParams color) {
	map<RGBClassIDs, double> ret_cutoff;
	switch (color) {
	case r:
		ret_cutoff = Rcutoff;
		break;
	case b:
		ret_cutoff = Bcutoff;
		break;
	case g:
		ret_cutoff = Gcutoff;
		break;
	default:
		Helpers::print_debug("ERROR: undefined color requested in SetCutoff; returning Rcutoff.\n");
		ret_cutoff = Rcutoff;
		break;
	}
	return ret_cutoff;
}
void OptiAlgo::FLSystem::ReassignCutoff(OutParams color, map<RGBClassIDs, double> cutoff_val) {
	switch (color) {
	case r:
		Rcutoff = cutoff_val;
		break;
	case b:
		Bcutoff = cutoff_val;
		break;
	case g:
		Gcutoff = cutoff_val;
		break;
	default:
		Helpers::print_debug("ERROR: undefined color requested in ReassignCutoff; returning Rcutoff.\n");
		Rcutoff = cutoff_val;
		break;
	}
}

LightsInfo OptiAlgo::FLSystem::Infer(AudioProps input, array<OutParams, 3> color_scheme) {
	LightsInfo out_crisp;
	vector<double> tnorm_temp;
	double temp;

	// Reset cutoffs before doing anything
	ResetCutoffs();

	// Use generic inference rules for testing
	map<RGBClassIDs, double> cutoff1, cutoff2, cutoff3;
	cutoff1 = SetCutoff(color_scheme[0]);
	cutoff2 = SetCutoff(color_scheme[1]);
	cutoff3 = SetCutoff(color_scheme[2]);

	// RED
	tnorm_temp.clear();
	tnorm_temp.push_back(FreqInClass(input.freq_avg, vlow));
	tnorm_temp.push_back(IntensInClass(input.loudness_avg, loud));
	cutoff1[strong] = Tnorm(tnorm_temp);

	tnorm_temp.clear();
	tnorm_temp.push_back(FreqInClass(input.freq_avg, low));
	tnorm_temp.push_back(IntensInClass(input.loudness_avg, loud));
	cutoff1[medium] = Tnorm(tnorm_temp);

	tnorm_temp.clear();
	tnorm_temp.push_back(max(FreqInClass(input.freq_avg, vlow), FreqInClass(input.freq_avg, low)));
	tnorm_temp.push_back(IntensInClass(input.loudness_avg, mid));
	cutoff1[dark] = Tnorm(tnorm_temp);

	tnorm_temp.clear();
	tnorm_temp.push_back(max(max(FreqInClass(input.freq_avg, high), FreqInClass(input.freq_avg, vhigh)),
								 IntensInClass(input.loudness_avg, quiet)) );
	cutoff1[none] = Tnorm(tnorm_temp);


	// BLUE
	tnorm_temp.clear();
	tnorm_temp.push_back(FreqInClass(input.freq_avg, vhigh));
	tnorm_temp.push_back(IntensInClass(input.loudness_avg, loud));
	cutoff2[strong] = Tnorm(tnorm_temp);

	tnorm_temp.clear();
	tnorm_temp.push_back(FreqInClass(input.freq_avg, high));
	tnorm_temp.push_back(IntensInClass(input.loudness_avg, loud));
	cutoff2[medium] = Tnorm(tnorm_temp);

	tnorm_temp.clear();
	tnorm_temp.push_back(max(FreqInClass(input.freq_avg, high), FreqInClass(input.freq_avg, vhigh)));
	tnorm_temp.push_back(IntensInClass(input.loudness_avg, mid));
	cutoff2[dark] = Tnorm(tnorm_temp);

	tnorm_temp.clear();
	tnorm_temp.push_back(max(max(FreqInClass(input.freq_avg, vlow), FreqInClass(input.freq_avg, low)),
								 IntensInClass(input.loudness_avg, quiet)));
	cutoff2[none] = Tnorm(tnorm_temp);


	// GREEN
	tnorm_temp.clear();
	tnorm_temp.push_back(FreqInClass(input.freq_avg, vhigh));
	tnorm_temp.push_back(TempoInClass(input.tempo_avg, vfast));
	tnorm_temp.push_back(IntensInClass(input.loudness_avg, loud));
	cutoff3[strong] = Tnorm(tnorm_temp);

	tnorm_temp.clear();
	tnorm_temp.push_back(max(FreqInClass(input.freq_avg, high), FreqInClass(input.freq_avg, vhigh)));
	tnorm_temp.push_back(TempoInClass(input.tempo_avg, fast));
	tnorm_temp.push_back(IntensInClass(input.loudness_avg, loud));
	temp = Tnorm(tnorm_temp);
	tnorm_temp.clear();
	tnorm_temp.push_back(FreqInClass(input.freq_avg, high));
	tnorm_temp.push_back(TempoInClass(input.tempo_avg, vfast));
	tnorm_temp.push_back(IntensInClass(input.loudness_avg, loud));
	temp = max(temp, Tnorm(tnorm_temp));
	tnorm_temp.clear();
	tnorm_temp.push_back(max(FreqInClass(input.freq_avg, low), FreqInClass(input.freq_avg, vlow)));
	tnorm_temp.push_back(max(TempoInClass(input.tempo_avg, fast), TempoInClass(input.tempo_avg, vfast)));
	tnorm_temp.push_back(IntensInClass(input.loudness_avg, loud));
	cutoff3[medium] = max(temp, Tnorm(tnorm_temp));

	tnorm_temp.clear();
	tnorm_temp.push_back(max(FreqInClass(input.freq_avg, high), FreqInClass(input.freq_avg, vhigh)));
	tnorm_temp.push_back(max(TempoInClass(input.tempo_avg, fast), TempoInClass(input.tempo_avg, vfast)));
	tnorm_temp.push_back(IntensInClass(input.loudness_avg, mid));
	temp = Tnorm(tnorm_temp);
	tnorm_temp.clear();
	tnorm_temp.push_back(max(FreqInClass(input.freq_avg, low), FreqInClass(input.freq_avg, vlow)));
	tnorm_temp.push_back(max(TempoInClass(input.tempo_avg, fast), TempoInClass(input.tempo_avg, vfast)));
	tnorm_temp.push_back(IntensInClass(input.loudness_avg, mid));
	cutoff3[dark] = max(temp, Tnorm(tnorm_temp));

	tnorm_temp.clear();
	tnorm_temp.push_back(max(max(max(TempoInClass(input.tempo_avg, vslow), TempoInClass(input.tempo_avg, slow)), TempoInClass(input.tempo_avg, moderate)),
		IntensInClass(input.loudness_avg, quiet)));
	cutoff3[none] = Tnorm(tnorm_temp);



	ReassignCutoff(color_scheme[0], cutoff1);
	ReassignCutoff(color_scheme[1], cutoff2);
	ReassignCutoff(color_scheme[2], cutoff3);

	out_str << "Rcutoff: ";
	for (int i = 0; i < 4; i++) {
		out_str << to_string((RGBClassIDs)i) << ": " << Rcutoff[(RGBClassIDs)i] << "; ";
	}
	out_str << "\n";
	out_str << "Bcutoff: ";
	for (int i = 0; i < 4; i++) {
		out_str << to_string((RGBClassIDs)i) << ": " << Bcutoff[(RGBClassIDs)i] << "; ";
	}
	out_str << "\n";
	out_str << "Gcutoff: ";
	for (int i = 0; i < 4; i++) {
		out_str << to_string((RGBClassIDs)i) << ": " << Gcutoff[(RGBClassIDs)i] << "; ";
	}
	out_str << "\n";


	if (concert_mode) {
		if (allow_strobing) {
			if (strobing_speed == 0) {
				// For strobing, just using speed as tempo_avg causes a lag of a 16th note (quarter of a beat)
				// (instead of falling on the next 'ta', it falls on the 'ka' after that, in the ta-ka-di-mi system)
				// Dividing by 5 gives you one 16th note; multiplying by 4 sets it for one quarter note (one beat)
				strobing_speed = (int)(4 * 84.0 / 5);
				while (strobing_speed <= 255) strobing_speed *= 2; // Double it to make strobing look nice
				strobing_speed /= 2; // Compensate for overshoot
			}
			out_crisp.strobing_speed = (int)strobing_speed;
		}
		else {
			out_crisp.strobing_speed = 0;
			strobing_speed = 0.0;
		}
	}
	else if (allow_strobing) {
		if (input.beatiness_avg > BEATINESS_LOUDNESS_THRESH && max(TempoInClass(input.tempo_avg, fast), TempoInClass(input.tempo_avg, vfast)) > BEATINESS_TEMPO_THRESH) {
			if (isStrobing <= 0) {
				// For strobing, just using speed as tempo_avg causes a lag of a 16th note (quarter of a beat)
				// (instead of falling on the next 'ta', it falls on the 'ka' after that, in the ta-ka-di-mi system)
				// Dividing by 5 gives you one 16th note; multiplying by 4 sets it for one quarter note (one beat)
				strobing_speed = (int)(4 * input.tempo_avg / 5);
				while (strobing_speed <= 255) strobing_speed *= 2; // Double it to make strobing look nice
				strobing_speed /= 2; // Compensate for overshoot
			}
			out_crisp.strobing_speed = (int)strobing_speed;
			isStrobing = STROBING_COOLDOWN;
		}
		else {
			if (isStrobing > 0) isStrobing--;
			else strobing_speed = 0;
		}
	}


	// Defuzzify each output parameter
	out_crisp.red_intensity = (int)Defuzzify(r);
	out_crisp.blue_intensity = (int)Defuzzify(b);
	out_crisp.green_intensity = (int)Defuzzify(g);
	out_crisp.white_intensity = 0;
	out_crisp.dimness = 255;

	// Done!
	return out_crisp;
}

#pragma endregion

#pragma region OptiAlgo

static queue<AudioInfo> audio_buffer = queue<AudioInfo>();
static queue<SectionInfo> song_section_buffer = queue<SectionInfo>();

bool OptiAlgo::concert_mode = false;
bool OptiAlgo::allow_strobing = false;

OptiAlgo::OptiAlgo()
{
	audio_buffer = queue<AudioInfo>();
	song_section_buffer = queue<SectionInfo>();

	current_section = none;

	color_scheme[0] = r;
	color_scheme[1] = b;
	color_scheme[2] = g;

	concert_mode = false;
	allow_strobing = false;
	terminate = false;
}

bool OptiAlgo::receive_audio_input_sample(AudioInfo audio_sample)
{
	if (audio_buffer.size() < AUDIO_BUF_SIZE)
		audio_buffer.push(audio_sample);
	return audio_buffer.size() < AUDIO_BUF_SIZE;
}

bool OptiAlgo::receive_song_section(SectionInfo song_section)
{
	if (song_section_buffer.size() < AUDIO_BUF_SIZE)
		song_section_buffer.push(song_section);
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
	AudioProps input_prev_section = AudioProps();
	AudioProps temp;
	SectionInfo new_section;
	double cur_freq, cur_tempo, cur_loud;
	bool got_freq, got_tempo, got_loud;
	int nudges_to_silence = NUDGES_TO_SILENCE;
	bool starting_silence = true;
	int max_loud_insertion_cooldown = MAX_LOUD_INSERTION_COOLDOWN;
	// Initialize solution variables
	FLSystem flSystem = FLSystem();
	LightsInfo cur_sol;
	// Initialize output variables
	deque<LightsInfo> out_hist = deque<LightsInfo>();
	deque<LightsInfo> delta_out_hist = deque<LightsInfo>();
	LightsInfo avg_out = LightsInfo(true);
	LightsInfo old_avg_out;
	// Initialize logging variables
	bool harmonicDetected = false;
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

			// Switch input context if section changes
			if (concert_mode && !song_section_buffer.empty()) {
				new_section = song_section_buffer.front();
				if (new_section.section != current_section) {
					Helpers::print_debug("[FL] Detected section change; switching input context.\n");

					current_section = new_section.section;
					allow_strobing = new_section.should_strobe;

					/*temp = input;
					input = input_prev_section;
					input_prev_section = AudioProps(temp);*/
				}
				song_section_buffer.pop();
			}

			// Are we still silent?
			if (got_loud) {
				if (cur_loud < SILENCE_THRESH) {
					if (!starting_silence) {
						input.silence = (nudges_to_silence <= 0);
						nudges_to_silence--;
					}
				}
				else {
					input.silence = false;
					starting_silence = false;
					nudges_to_silence = NUDGES_TO_SILENCE;
				}
			}

			// Update averages & history
			if (!input.silence)
			{
				if (got_freq) {
					// Check for harmonic frequencies and correct them before inserting
					harmonicDetected = false;
					if (input.freq_hist.size() >= IN_HIST_BUF_SIZE && FREQ_ANOMALY_NUDGES_TO_CHANGE > 0 &&
						(abs(cur_freq - input.freq_avg * 2) < FREQ_HARMONIC_DETECT_THRESH || abs(cur_freq - input.freq_avg / 2) < FREQ_HARMONIC_DETECT_THRESH))
					{
						harmonicDetected = true;
						input.delta_harmonic_freq_add_and_avg(cur_freq);
						if (input.delta_harmonic_freq_hist.size() >= FREQ_HARMONIC_NUDGES_TO_CHANGE) {
							input.freq_hist = input.delta_harmonic_freq_hist;
							input.freq_avg = input.delta_harmonic_freq_avg;
							input.delta_harmonic_freq_hist.clear();
						}
						input.delta_anomaly_freq_hist.clear();
					}
					// Check for frequency anomalies and correct them before inserting
					else if (input.freq_hist.size() >= IN_HIST_BUF_SIZE && FREQ_ANOMALY_NUDGES_TO_CHANGE > 0 &&
							abs(cur_freq - input.freq_avg) > FREQ_ANOMALY_DETECT_THRESH)
					{
						input.delta_anomaly_freq_add_and_avg(cur_freq);
						if (input.delta_anomaly_freq_hist.size() >= FREQ_ANOMALY_NUDGES_TO_CHANGE) {
							input.freq_hist = input.delta_anomaly_freq_hist;
							input.freq_avg = input.delta_anomaly_freq_avg;
							input.delta_anomaly_freq_hist.clear();
						}
						input.delta_harmonic_freq_hist.clear();
					}
					else {
						input.freq_add_and_avg(cur_freq);
						input.delta_harmonic_freq_hist.clear();
						input.delta_anomaly_freq_hist.clear();
					}
				}

				if (got_tempo) {
					if (input.tempo_hist.size() > 0 && abs(cur_tempo - input.tempo_avg) > TEMPO_CLOSENESS_THRESH) {
						input.delta_tempo_add_and_avg(cur_tempo);
						if (input.delta_tempo_hist.size() >= TEMPO_NUDGES_TO_CHANGE) {
							input.tempo_hist.clear();
							input.tempo_add_and_avg(cur_tempo);
							//input.tempo_avg = input.delta_tempo_avg;
							input.delta_tempo_hist.clear();
						}
					}
					else {
						input.tempo_add_and_avg(cur_tempo);
					}
				}

				if (got_loud)
				{
					input.loudness_hist_add_and_avg(cur_loud);
					if (input.loudness_nomax_hist.size() > 0 && cur_loud - input.loudness_nomax_avg >= MAX_LOUDNESS_THRESH) {
						input.loudness_max_hist_add_and_avg(cur_loud);
						max_loud_insertion_cooldown = (max_loud_insertion_cooldown < MAX_LOUD_INSERTION_COOLDOWN) ? max_loud_insertion_cooldown + 1 : MAX_LOUD_INSERTION_COOLDOWN;
					}
					else {
						if (max_loud_insertion_cooldown > 0) max_loud_insertion_cooldown--;
						else {
							input.loudness_max_hist_add_and_avg(input.loudness_avg);
						}
						input.loudness_nomax_hist_add_and_avg(cur_loud);
					}
					input.beatiness_hist_add_and_avg(input.loudness_max_avg - input.loudness_nomax_avg);
				}
			}

			// Fuzzy inference
			if (!input.silence) {
				// Use fuzzy logic system to get output
				cur_sol = flSystem.Infer(input, color_scheme);

				// Average output
				if (out_hist.size() >= OUT_HIST_BUF_SIZE && (
						abs(cur_sol.red_intensity - old_avg_out.red_intensity) > OUTPUT_ANOMALY_DETECT_THRESH ||
						abs(cur_sol.blue_intensity - old_avg_out.blue_intensity) > OUTPUT_ANOMALY_DETECT_THRESH ||
						abs(cur_sol.green_intensity - old_avg_out.green_intensity) > OUTPUT_ANOMALY_DETECT_THRESH)) {
					delta_out_hist.push_front(cur_sol);
					if (delta_out_hist.size() >= OUTPUT_NUDGES_TO_CHANGE) {
						delta_out_hist.pop_front();
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
			out_str << "in_avg:[F, T, L (beat)]=["
				<< input.freq_avg;
			if (harmonicDetected) out_str << "*";
			out_str << ", " << cur_tempo << "->" << input.tempo_avg << ", "
				<< input.loudness_avg << " (" << input.loudness_max_avg << " - " << input.loudness_nomax_avg << " = "
				<< (input.loudness_max_avg - input.loudness_nomax_avg) << " -> " << input.beatiness_avg << ")]";

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

void OptiAlgo::start(bool song_selected, bool auto_strobe = false)
{
	concert_mode = song_selected;
	if (song_selected) auto_strobe = false;
	allow_strobing = auto_strobe;

	start_algo();
	//test_lights();
}

void OptiAlgo::start(bool song_selected, array<OutParams, 3> color_scheme, bool auto_strobe = false)
{
	this->color_scheme = color_scheme;

	start(song_selected, auto_strobe);
}

void OptiAlgo::stop()
{
	terminate = true;
}

#pragma endregion