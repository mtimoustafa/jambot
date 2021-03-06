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
#include "Constructs.h"
#include "Constants.h"
using namespace std;


class OptiAlgo
{
	class AudioProps
	{
		deque<double> freq_conc_hist = deque<double>();
		deque<double> overall_tempo_hist = deque<double>();
		deque<double> beatiness_hist = deque<double>();
		deque<double> overall_intens_hist = deque<double>();

	public:
		pair<map<string, double>, double> freq_conc;
		pair<map<string, double>, double> overall_tempo;
		pair<map<string, double>, double> beatiness;
		pair<map<string, double>, double> overall_intens;
		double silence = 1;

		AudioProps();
		map<string, double> new_modifiers_set(const double * mods);
		void adjust_weights(AudioInfo input, double max_loud);

		double freq_conc_add_and_avg(double val);
		double overall_tempo_add_and_avg(double val);
		double beatiness_add_and_avg(double val);
		double overall_intens_add_and_avg(double val);
	};

	class ProblemRepresentation
	{
	public:
		LightsInfo representation;
		double rep_value;
		double tune_ub = TUNE_UB;
		double tune_lb = TUNE_LB;

		ProblemRepresentation();
		ProblemRepresentation(AudioProps properties, bool centered);

		double objective_function(LightsInfo cand_sol, AudioProps props);
		double obf_overall_frequency(LightsInfo cand_sol, AudioProps props);
		double obf_overall_tempo(LightsInfo cand_sol, AudioProps props);
		double obf_beatiness(LightsInfo cand_sol, AudioProps props);
		double obf_overall_intens(LightsInfo cand_sol, AudioProps props);
		double obf_silence(LightsInfo cand_sol, AudioProps props);

		int tune(int value);
	};

	class TabuSearch
	{
		int TENURE;
		int n_iterations;

	public:
		TabuSearch(int tenure, int n_iterations);

		static bool pairCompare(const pair<LightsInfo, double>& firstElem, const pair<LightsInfo, double>& secondElem);
		ProblemRepresentation search(ProblemRepresentation problem, AudioProps audio_props);
	};

	AudioProps audio_props;
	bool terminate;

public:
	OptiAlgo();

	static bool receive_audio_input_sample(AudioInfo audio_sample); // returns false if internal buffer is full
	map<string, double> execute_algorithm(TabuSearch algo, int n_iterations);
	void test_algo();
	void test_lights();
	void start_algo();
	void start();
	void stop();
};

#endif