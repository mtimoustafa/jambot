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

#include "Constructs.h"
#include "Constants.h"
#include "Helpers.h"
#include "OptiAlgo.h"

using namespace std;

map<string, double> new_modifiers_set(const double * mods)
{
	map<string, double> modifiers;
	modifiers.insert(make_pair("r_intens", mods[0]));
	modifiers.insert(make_pair("g_intens", mods[1]));
	modifiers.insert(make_pair("b_intens", mods[2]));
	modifiers.insert(make_pair("w_intens", mods[3]));
	modifiers.insert(make_pair("saturation", mods[4]));
	modifiers.insert(make_pair("dimness", mods[5]));
	modifiers.insert(make_pair("strobe_rate", mods[6]));
	modifiers.insert(make_pair("strobe_str", mods[7]));
	return modifiers;
}

class AudioProps
{
public:
	pair<map<string, double>, double> freq_conc;
	pair<map<string, double>, double> pace;
	pair<map<string, double>, double> beatiness;
	pair<map<string, double>, double> overall_intens;



	AudioProps()
	{
		freq_conc = make_pair(new_modifiers_set(FREQ_MODS), 0.2);
		pace = make_pair(new_modifiers_set(PACE_MODS), 0.4);
		beatiness = make_pair(new_modifiers_set(BEATI_MODS), 0.1);
		overall_intens = make_pair(new_modifiers_set(OV_INT_MODS), 0.3);
	}
};

AudioProps properties = AudioProps();

#pragma region ProblemRepresentation

OptiAlgo::ProblemRepresentation::ProblemRepresentation()
{
	// Start with a random candidate solution
	representation = LightsInfo();
	rep_value = objective_function(representation);
}

double OptiAlgo::ProblemRepresentation::objective_function(LightsInfo cand_sol)
{
	// Applies objective function to current solution
	return
		properties.freq_conc.second * (
		1 / (1 + abs((double)cand_sol.red_intensity - properties.freq_conc.first["r_intens"]))
		- 1 / (1 + abs((double)cand_sol.blue_intensity - properties.freq_conc.first["b_intens"]))
		+ 1 / (1 + abs((double)cand_sol.green_intensity - properties.freq_conc.first["g_intens"]))
		- 1 / (1 + abs((double)cand_sol.white_intensity - properties.freq_conc.first["w_intens"]))) +
		properties.pace.second * (
		1 / (1 + abs((double)cand_sol.blue_intensity - properties.pace.first["b_intens"]))
		- 1 / (1 + abs((double)cand_sol.green_intensity - properties.pace.first["g_intens"]))
		+ 1 / (1 + abs((double)cand_sol.white_intensity - properties.pace.first["w_intens"]))
		- 1 / (1 + abs((double)cand_sol.strobing_speed - properties.pace.first["strobe_rate"]))) +
		properties.beatiness.second * (
		1 / (1 + abs((double)cand_sol.green_intensity - properties.beatiness.first["g_intens"]))
		- 1 / (1 + abs((double)cand_sol.white_intensity - properties.beatiness.first["w_intens"]))
		+ 1 / (1 + abs((double)cand_sol.strobing_speed - properties.beatiness.first["strobe_rate"]))
		- 1 / (1 + abs((double)cand_sol.dimness - properties.beatiness.first["dimness"]))) +
		properties.overall_intens.second * (
		1 / (1 + abs((double)cand_sol.white_intensity - properties.beatiness.first["w_intens"]))
		- 1 / (1 + abs((double)cand_sol.strobing_speed - properties.beatiness.first["strobe_rate"]))
		+ 1 / (1 + abs((double)cand_sol.dimness - properties.beatiness.first["dimness"]))
		- 1 / (1 + abs((double)cand_sol.red_intensity - properties.beatiness.first["r_intens"])));
}

LightsInfo OptiAlgo::ProblemRepresentation::tune()
{
	// Randomly alter variables
	LightsInfo tuned_rep = LightsInfo();
	tuned_rep.red_intensity += (int)Helpers::fRand(tune_lb, tune_ub);
	tuned_rep.blue_intensity += (int)Helpers::fRand(tune_lb, tune_ub);
	tuned_rep.green_intensity += (int)Helpers::fRand(tune_lb, tune_ub);
	tuned_rep.white_intensity += (int)Helpers::fRand(tune_lb, tune_ub);
	tuned_rep.strobing_speed += (int)Helpers::fRand(tune_lb, tune_ub);
	tuned_rep.dimness += (int)Helpers::fRand(tune_lb, tune_ub);
	return tuned_rep;
}

#pragma endregion


#pragma region TabuSearch

OptiAlgo::TabuSearch::TabuSearch(int tenure = 3, int n_iterations = 1000)
{
	TENURE = tenure;
	this->n_iterations = n_iterations;
}

// Helper function
bool OptiAlgo::TabuSearch::pairCompare(const pair<LightsInfo, double>& firstElem, const pair<LightsInfo, double>& secondElem) {
	return firstElem.second > secondElem.second;

}

OptiAlgo::ProblemRepresentation OptiAlgo::TabuSearch::search(ProblemRepresentation problem)
{
	// Initialize structures
	vector<pair<LightsInfo, double>> neighbours;
	vector<pair<LightsInfo, int>> tabuStructure;
	vector<pair<LightsInfo, int>>::const_iterator it;

	LightsInfo temp_neighbour;
	pair<LightsInfo, double> candidate;
	bool solutionChanged;
	vector<pair<LightsInfo, int>> new_structure;

	// debug values - TODO: remove these
	bool res;
	LightsInfo dbg;
	vector<pair<LightsInfo, double>> dbgn;

	// Search for optimal solution!
	for (int i = 0; i < n_iterations; i++)
	{
		// Generate neighbours
		neighbours = vector<pair<LightsInfo, double>>();
		for (int j = 0; j < 5; j++)
		{
			temp_neighbour = problem.tune();
			neighbours.push_back(make_pair(temp_neighbour, (problem.objective_function(temp_neighbour) - problem.objective_function(problem.representation))));
		}
		sort(neighbours.begin(), neighbours.end(), TabuSearch::pairCompare);
		dbgn = neighbours;

		// Choose new candidate solution
		solutionChanged = false;
		while (neighbours.size() > 0)
		{
			candidate = neighbours.front();
			neighbours.erase(neighbours.begin());

			// If not tabu or if aspiration criteria allows it
			it = find_if(tabuStructure.begin(), tabuStructure.end(), [&candidate, &res, &dbg](const pair<LightsInfo, double> &x) {
				dbg = x.first;
				res = x.first == candidate.first;
				return x.first == candidate.first; });
				if (it == tabuStructure.end() || (it->second <= 1 && !neighbours.empty() && candidate.second >= neighbours.front().second * 2))
				{
					// Use neighbouring solution
					problem.representation = candidate.first;
					problem.rep_value += candidate.second;

					if (it != tabuStructure.end())
					{
						tabuStructure.erase(it);
					}
					tabuStructure.push_back(make_pair(problem.representation, TENURE + 1));

					solutionChanged = true;
					break;
				}
		}
		if (!solutionChanged)
		{
			cerr << "x";
		}

		// Decrement values in tabu structure
		new_structure = vector<pair<LightsInfo, int>>();
		for each (pair<LightsInfo, int> element in tabuStructure)
		{
			if (element.second > 0)
			{
				element.second--;
				new_structure.push_back(element);
			}
		}
		tabuStructure = new_structure;
	}

	return problem;
}

#pragma endregion


OptiAlgo::OptiAlgo()
{
	srand(static_cast<unsigned int>(time(NULL)));
	audio_buffer = queue<AudioInfo>();
}

bool OptiAlgo::receive_audio_input_sample(AudioInfo audio_sample)
{
	if (audio_buffer.size() < AUDIO_BUF_SIZE)
		audio_buffer.push(audio_sample);
	return audio_buffer.size() < AUDIO_BUF_SIZE;
}

map<string, double> OptiAlgo::execute_algorithm(TabuSearch algo, int n_iterations)
{
	double sum;
	double sum_time;
	clock_t begin, end;
	list<double> values;
	ProblemRepresentation problem;
	double mean, variance;

	// Execute search multiple times, compute statistics
	sum = 0;
	sum_time = 0.0;
	for (int i = 0; i < n_iterations; i++)
	{
		problem = ProblemRepresentation();
		begin = clock();
		problem = algo.search(problem);
		end = clock();
		values.push_back(problem.rep_value);
		sum += problem.rep_value;
		sum_time += double(end - begin) / CLOCKS_PER_SEC;
	}

	mean = sum / n_iterations;
	variance = 0.0;
	for each (double val in values)
	{
		variance += (mean - val) * (mean - val);
	}
	variance /= n_iterations;

	map<string, double> stats = map<string, double>();
	stats.insert(make_pair("max", *(max_element(values.begin(), values.end()))));
	stats.insert(make_pair("mean", mean));
	stats.insert(make_pair("var", variance));
	stats.insert(make_pair("avg", sum_time / n_iterations * 1000));

	return stats;
}

void OptiAlgo::test_algo()
{
	ofstream tabuFile;

	// Apply Tabu search
	TabuSearch tabuSearch;
	map<string, double> stats;

	tabuFile.open("tabu_results.csv");
	tabuFile << "tenure;N_iterations;Mean;Max;Variance;Avg_time\n";
	Helpers::print_debug("Testing Best-fit Tabu search Optimization Algorithm:");
	for (int n_iterations = 1; n_iterations <= 100; n_iterations = n_iterations==100 ? 150 : n_iterations * 10)
	{
		Helpers::print_debug(("\n" + to_string(n_iterations)).c_str());
		for (int tenure = 1; tenure < 20; tenure++)
		{
			Helpers::print_debug(".");
			tabuSearch = TabuSearch(tenure, n_iterations);
			stats = execute_algorithm(tabuSearch, 100);
			tabuFile << tenure << ";" << n_iterations << ";" << stats.at("mean") << ";"
				<< stats.at("max") << ";" << stats.at("var") << ";" << stats.at("avg") << "\n";
		}
	}
	Helpers::print_debug("\ntest done. Results output to tabu_results.csv");
	tabuFile.close();
}

void OptiAlgo::start()
{
	unsigned int tenure = 5, n_iterations = 150;
	TabuSearch algorithm = TabuSearch(tenure, n_iterations);
	AudioInfo audio_sample;
	ProblemRepresentation solution;
	queue<ProblemRepresentation> sample_history; // TODO: incorporate more of history in smoothing and change
	unsigned int nudges;

	while (true) // TODO: stop when song ends
	{
		// Wait for audio input samples
		while (audio_buffer.empty()) {}

		// Find solution to audio sample
		audio_sample = audio_buffer.front();

		// TODO: score properties according to audio sample

		// Use properties to find varying, near-optimal lights configuration
		solution = algorithm.search(solution);

		// Smooth solution
		if (solution.representation.is_similar_to(sample_history.back().representation))
			solution.representation = sample_history.back().representation;

		// Trigger change
		if (sample_history.back().representation == solution.representation) nudges++;
		else nudges--;

		if (nudges >= NUDGES_TO_CHANGE)
		{

		}

		// Update history
		if (sample_history.size() >= HISTORY_BUF_SIZE)
			sample_history.pop();
		sample_history.push(solution);

		//TODO: give_jack(lightsInfo);

		// Last step: remove analysed sample from buffer
		audio_buffer.pop();
	}
}
