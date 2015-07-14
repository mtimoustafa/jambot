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
#include "Constructs.h"
#include "Constants.h"
using namespace std;


class OptiAlgo
{
public:
	OptiAlgo();

	class ProblemRepresentation
	{
	public:
		LightsInfo representation;
		double rep_value;
		double tune_ub = TUNE_UB;
		double tune_lb = TUNE_LB;

		ProblemRepresentation();

		double objective_function(LightsInfo cand_sol);

		LightsInfo tune();
	};

	class TabuSearch
	{
		int TENURE;
		int n_iterations;

	public:
		TabuSearch(int tenure, int n_iterations);

		static bool pairCompare(const pair<LightsInfo, double>& firstElem, const pair<LightsInfo, double>& secondElem);
		ProblemRepresentation search(ProblemRepresentation problem);
	};

	map<string, double> execute_algorithm(TabuSearch algo, int n_iterations);
	void test_algo();
	void start();

};

#endif