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
	public:
		pair<map<string, double>, double> freq_conc;
		pair<map<string, double>, double> pace;
		pair<map<string, double>, double> beatiness;
		pair<map<string, double>, double> overall_intens;

		AudioProps();
		map<string, double> new_modifiers_set(const double * mods);
	};

	class ProblemRepresentation
	{
	public:
		LightsInfo representation;
		double rep_value;
		double tune_ub = TUNE_UB;
		double tune_lb = TUNE_LB;

		ProblemRepresentation();
		ProblemRepresentation(AudioProps properties);

		double objective_function(LightsInfo cand_sol, AudioProps properties);

		LightsInfo tune();
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

	queue<AudioInfo> audio_buffer;
	AudioProps audio_props;

public:
	OptiAlgo();

	bool receive_audio_input_sample(AudioInfo audio_sample); // returns false if internal buffer is full
	map<string, double> execute_algorithm(TabuSearch algo, int n_iterations);
	void test_algo();
	void start();

};

#endif