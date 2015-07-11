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

#include "Constructs.h"
#include "Constants.h"
#include "Helpers.h"
#include "BestFitOptimizationAlgorithm.h"
using namespace std;


#pragma region ProblemRepresentation

ProblemRepresentation::ProblemRepresentation()
{
	// Start with a random candidate solution
	representation = AudioInfo();
	rep_value = objective_function(representation);
}

double ProblemRepresentation::objective_function(AudioInfo cand_sol)
{
	// Applies objective function to current solution
	double freq, loud, tempo;
	cand_sol.get_frequency(freq);
	cand_sol.get_loudness(loud);
	cand_sol.get_tempo(tempo);
	return 0.5*(freq * 1 - loud * 2 + tempo * 3) + 0.3*(loud * 1 - tempo * 2 + freq * 3) + 0.2*(tempo * 1 - freq * 2 + loud * 3);
}

AudioInfo ProblemRepresentation::tune()
{
	// Randomly alter variables
	AudioInfo tuned_rep = AudioInfo();
	double freq, loud, tempo;
	representation.get_frequency(freq);
	representation.get_loudness(loud);
	representation.get_tempo(tempo);

	tuned_rep.set_frequency(freq + Helpers::fRand(tune_lb, tune_ub));
	tuned_rep.set_loudness(loud + Helpers::fRand(tune_lb, tune_ub));
	tuned_rep.set_tempo(tempo + Helpers::fRand(tune_lb, tune_ub));

	return tuned_rep;
}

void ProblemRepresentation::print_candidate_solution(AudioInfo cand_sol)
{
	double freq, loud, tempo;
	cand_sol.get_frequency(freq);
	cand_sol.get_loudness(loud);
	cand_sol.get_tempo(tempo);
	Helpers::print_debug(("F=" + to_string(freq) + " L=" + to_string(loud) + " T=" + to_string(tempo)+"\n").c_str());
}

#pragma endregion


#pragma region TabuSearch

TabuSearch::TabuSearch(int tenure = 3, int n_iterations = 1000)
{
	TENURE = tenure;
	this->n_iterations = n_iterations;
}

// Helper function
bool TabuSearch::pairCompare(const pair<AudioInfo, double>& firstElem, const pair<AudioInfo, double>& secondElem) {
	return firstElem.second > secondElem.second;

}

ProblemRepresentation TabuSearch::search(ProblemRepresentation problem)
{
	// Initialize structures
	vector<pair<AudioInfo, double>> neighbours;
	AudioInfo bestNeighbour;
	vector<pair<AudioInfo, int>> tabuStructure;
	vector<pair<AudioInfo, int>>::const_iterator it;

	AudioInfo temp_neighbour;
	double freq, loud, tempo;
	pair<AudioInfo, double> candidate;
	bool solutionChanged;
	vector<pair<AudioInfo, int>> new_structure;

	// debug values - TODO: remove these
	bool res;
	AudioInfo dbg;
	vector<pair<AudioInfo, double>> dbgn;

	// Search for optimal solution!
	for (int i = 0; i < n_iterations; i++)
	{
		// Generate neighbours
		neighbours = vector<pair<AudioInfo, double>>();
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
			it = find_if(tabuStructure.begin(), tabuStructure.end(), [&candidate, &res, &dbg](const pair<AudioInfo, double> &x) {
				dbg = x.first;
				res = x.first == candidate.first;
				return x.first == candidate.first; });
				if (it == tabuStructure.end() || (it->second <= 1 && !neighbours.empty() && candidate.second >= neighbours.front().second * 2))
				{
					// Use neighbouring solution
					candidate.first.get_frequency(freq);
					candidate.first.get_loudness(loud);
					candidate.first.get_tempo(tempo);
					problem.representation = AudioInfo(&freq, &loud, &tempo);
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
		new_structure = vector<pair<AudioInfo, int>>();
		for each (pair<AudioInfo, int> element in tabuStructure)
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
};

#pragma endregion


#pragma region BestFitOptimizationAlgorithm

BestFitOptimizationAlgorithm::BestFitOptimizationAlgorithm()
{
	srand(time(NULL));
}

map<string, double> BestFitOptimizationAlgorithm::execute_algorithm(TabuSearch algo, int n_iterations)
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

void BestFitOptimizationAlgorithm::start()
{
	ofstream tabuFile;

	// Apply Tabu search
	TabuSearch tabuSearch;
	map<string, double> stats;

	tabuFile.open("tabu_results.csv");
	tabuFile << "tenure;N_iterations;Mean;Max;Variance;Avg_time\n";
	Helpers::print_debug("Testing Best-fit Tabu search Optimization Algorithm:");
	for (int n_iterations = 1; n_iterations <= 100; n_iterations *= 10)
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

#pragma endregion