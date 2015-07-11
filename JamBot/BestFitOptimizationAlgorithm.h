#ifndef BESTFITOPTIMIZATIONALGORITHM_INCLUDE
#define BESTFITOPTIMIZATIONALGORITHM_INCLUDE

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
#include "Constructs.h"
using namespace std;

class ProblemRepresentation
{
public:
	AudioInfo representation;
	double rep_value;
	double tune_ub = 10.0;
	double tune_lb = -10.0;

	ProblemRepresentation();

	double objective_function(AudioInfo cand_sol);

	AudioInfo tune();

	void print_candidate_solution(AudioInfo cand_sol);
};


class TabuSearch
{
	int TENURE;
	int n_iterations;

public:
	TabuSearch(int tenure, int n_iterations);

	static bool pairCompare(const pair<AudioInfo, double>& firstElem, const pair<AudioInfo, double>& secondElem);
	ProblemRepresentation search(ProblemRepresentation problem);
};


class BestFitOptimizationAlgorithm
{
public:
	BestFitOptimizationAlgorithm();

	map<string, double> execute_algorithm(TabuSearch algo, int n_iterations);
	void start();
};

#endif