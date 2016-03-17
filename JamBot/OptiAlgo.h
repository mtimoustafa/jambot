#ifndef OPTIALGO_INCLUDE
#define OPTIALGO_INCLUDE

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
#include <queue>
#include <sstream>
#include "Constructs.h"
#include "Constants.h"
using namespace std;

class OptiAlgo
{
	class AudioProps
	{
	public:
		double silence = 1;

		double freq_avg;
		double delta_freq_avg;
		double tempo_avg;
		double loudness_avg;
		double loudness_nomax_avg;
		double loudness_max_avg;

		deque<double> freq_hist = deque<double>();
		deque<double> delta_freq_hist = deque<double>();
		deque<double> tempo_hist = deque<double>();
		deque<double> loudness_hist = deque<double>();
		deque<double> loudness_nomax_hist = deque<double>();
		deque<double> loudness_max_hist = deque<double>();

		AudioProps();
		void ClearProps();

		double freq_add_and_avg(double val);
		double delta_freq_add_and_avg(double val);
		double tempo_add_and_avg(double val);
		double loudness_hist_add_and_avg(double val);
		double loudness_nomax_hist_add_and_avg(double val);
		double loudness_max_hist_add_and_avg(double val);
	};

	class FLSystem
	{
		// Membership function classes:
		// Inputs
		enum FreqClassIDs { vlow, low, high, vhigh };
		enum TempoClassIDs { vslow, slow, moderate, fast, vfast };
		enum IntensClassIDs { quiet, mid, loud };
		enum BeatinessClassIDs { notbeaty, sbeaty, beaty, vbeaty };
		// Outputs
		enum RGBClassIDs { none, dark, medium, strong };
		enum WClassIDs { off, normal, bright };
		// Output parameters
		enum OutParams { r, g, b, w, dim, str };

		// Membership function ...er, functions:
		// Inputs
		double FreqInClass(double input, FreqClassIDs flClass);
		double TempoInClass(double input, TempoClassIDs flClass);
		double IntensInClass(double input, IntensClassIDs flClass);
		double BeatinessInClass(double input, BeatinessClassIDs flClass);
		// Outputs
		double ROutClass(double input, RGBClassIDs flClass);
		double GOutClass(double input, RGBClassIDs flClass);
		double BOutClass(double input, RGBClassIDs flClass);
		//double WOutClass(double input, WClassIDs flClass);

		// Cutoffs for computing modified output classes
		map <RGBClassIDs, double> Rcutoff;
		map <RGBClassIDs, double> Gcutoff;
		map <RGBClassIDs, double> Bcutoff;
		map <WClassIDs, double> Wcutoff;
		void ResetCutoffs();

		// Fuzzy operators
		double Tnorm(vector<double> inputs);
		double Defuzzify(OutParams outparam);

		// Local helper functions
		double Integrate(OutParams outparam, double lb, double ub, double step);
		double (OptiAlgo::FLSystem::*OutClass)(double, RGBClassIDs); // acompannying function pointer to Integrate()

	public:
		FLSystem();
		LightsInfo Infer(AudioProps input); // Runs the fuzzy logic system
	};

	bool terminate;

public:
	OptiAlgo();

	static bool receive_audio_input_sample(AudioInfo audio_sample); // returns false if internal buffer is full
	static bool receive_song_section(SECTION audio_sample); // returns false if internal buffer is full

	void test_lights();
	void start_algo();
	void start(bool);
	void stop();
};

#endif