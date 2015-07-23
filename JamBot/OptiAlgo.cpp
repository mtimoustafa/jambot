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

#include "Constructs.h"
#include "Constants.h"
#include "Helpers.h"
#include "OptiAlgo.h"
#include "DMXOutput.h"

using namespace std;


#pragma region AudioProps

OptiAlgo::AudioProps::AudioProps()
{
	freq_conc = make_pair(new_modifiers_set(FREQ_MODS), 0.5);
	overall_tempo = make_pair(new_modifiers_set(OV_TEMPO_MODS), 0.5);
	beatiness = make_pair(new_modifiers_set(BEATI_MODS), 0.5);
	overall_intens = make_pair(new_modifiers_set(OV_INT_MODS), 0.5);
}

map<string, pair<double, double>> OptiAlgo::AudioProps::new_modifiers_set(const double * mods)
{
	map<string, pair<double, double>> modifiers;
	modifiers.insert(make_pair("r", make_pair(mods[0],mods[1])));
	modifiers.insert(make_pair("g", make_pair(mods[2], mods[3])));
	modifiers.insert(make_pair("b", make_pair(mods[4], mods[5])));
	modifiers.insert(make_pair("w", make_pair(mods[6], mods[7]))); // saturation
	modifiers.insert(make_pair("dim", make_pair(mods[8], mods[9]))); // dimness
	modifiers.insert(make_pair("straff", make_pair(mods[10], mods[11]))); // strobe affinity
	modifiers.insert(make_pair("strstr", make_pair(mods[12], mods[13]))); // strobe strength
	return modifiers;
}

void OptiAlgo::AudioProps::adjust_weights(AudioInfo input, double max_loud)
{
	double freq, tempo, loud;
	input.get_frequency(freq);
	input.get_tempo(tempo);
	input.get_loudness(loud);

	// NT: set freq conc weight
	freq_conc.second = 0.0;
	overall_tempo.second = 0.5 * (loud / LOUD_UB) * (1 + tempo / TEMPO_UB); // NT: add freq to this weight
	beatiness.second = (max_loud - loud)/(LOUD_UB - loud); // NT: add freq to this weight
	overall_tempo.second = (tempo / TEMPO_UB);
}

#pragma endregion


#pragma region ProblemRepresentation

OptiAlgo::ProblemRepresentation::ProblemRepresentation()
{
	// Start with a random candidate solution
	representation = LightsInfo();
	rep_value = 0;
}
OptiAlgo::ProblemRepresentation::ProblemRepresentation(AudioProps properties)
{
	// Start with a random candidate solution
	representation = LightsInfo();
	rep_value = objective_function(representation, properties);
}

double OptiAlgo::ProblemRepresentation::objective_function(LightsInfo cand_sol, AudioProps props)
{
	// Applies objective function to current solution
	double obf_freq_conc, obf_overall_tempo, obf_beatiness, obf_overall_intens;
	double weight, r, g, b, w, dim, straff, strstr;
	double r_big, g_big, b_big;

	obf_freq_conc = 0.0; // NT: add freq conc portion of objective function

	weight = props.overall_tempo.second;
	r = props.overall_tempo.first["r"].first;
	r_big = props.overall_tempo.first["r"].second;
	b = props.overall_tempo.first["b"].first;
	b_big = props.overall_tempo.first["b"].second;
	g = props.overall_tempo.first["g"].first;
	g_big = props.overall_tempo.first["g"].second;
	b = props.overall_tempo.first["b"].second;
	w = props.overall_tempo.first["w"].second;
	obf_overall_tempo = 1 / (abs(b_big + (b - b_big)*weight - (double)cand_sol.blue_intensity) + 1) +
		1 / (abs(weight*r + (double)cand_sol.red_intensity) + 1) +
		1 / (abs(weight*g + (double)cand_sol.green_intensity) + 1) +
		1 / (abs(weight*w + (double)cand_sol.white_intensity) + 1); // NT: add strobing stuff

	weight = props.beatiness.second;
	r = props.beatiness.first["r"].first;
	r_big = props.beatiness.first["r"].second;
	g = props.beatiness.first["g"].first;
	g_big = props.beatiness.first["g"].second;
	b = props.beatiness.first["b"].second;
	dim = props.beatiness.first["dim"].second;
	obf_beatiness = 1 / (abs(g_big + (g - g_big)*weight - (double)cand_sol.green_intensity) + 1) +
		1 / (abs(r_big + (r - r_big)*weight - (double)cand_sol.red_intensity) + 1) +
		1 / (abs(weight*b + (double)cand_sol.blue_intensity) + 1) +
		1 / (abs(weight*dim + (double)cand_sol.dimness) + 1); // NT: add strobing stuff

	weight = props.overall_intens.second;
	g = props.overall_intens.first["g"].first;
	w = props.overall_intens.first["w"].first;
	dim = props.beatiness.first["dim"].second;
	obf_overall_intens = 1 / (abs(weight*g + (double)cand_sol.green_intensity) + 1) +
		1 / (abs(weight*w + (double)cand_sol.white_intensity) + 1) +
		1 / (abs(weight*dim + (double)cand_sol.dimness) + 1); // NT: add strobing stuff

	return obf_freq_conc + obf_overall_tempo + obf_beatiness + obf_overall_intens;
}

LightsInfo OptiAlgo::ProblemRepresentation::tune(LightsInfo tuned_rep)
{
	// Randomly alter variables
	//LightsInfo tuned_rep = LightsInfo();
	tuned_rep.red_intensity += (int)Helpers::fRand(tune_lb, tune_ub);
	if (tuned_rep.red_intensity > (int)R_UB) tuned_rep.red_intensity = (int)R_UB;
	if (tuned_rep.red_intensity < (int)R_LB) tuned_rep.red_intensity = (int)R_LB;

	tuned_rep.blue_intensity += (int)Helpers::fRand(tune_lb, tune_ub);
	if (tuned_rep.blue_intensity > (int)B_UB) tuned_rep.blue_intensity = (int)B_UB;
	if (tuned_rep.blue_intensity < (int)B_LB) tuned_rep.blue_intensity = (int)B_LB;

	tuned_rep.green_intensity += (int)Helpers::fRand(tune_lb, tune_ub);
	if (tuned_rep.green_intensity > (int)G_UB) tuned_rep.green_intensity = (int)G_UB;
	if (tuned_rep.green_intensity < (int)G_LB) tuned_rep.green_intensity = (int)G_LB;

	tuned_rep.white_intensity += (int)Helpers::fRand(tune_lb, tune_ub);
	if (tuned_rep.white_intensity > (int)W_UB) tuned_rep.white_intensity = (int)W_UB;
	if (tuned_rep.white_intensity < (int)W_LB) tuned_rep.white_intensity = (int)W_LB;

	// tuned_rep.strobing_speed += (int)Helpers::fRand(tune_lb, tune_ub);
	tuned_rep.strobing_speed = 0; // NT: add this in

	tuned_rep.dimness += (int)Helpers::fRand(tune_lb, tune_ub);
	if (tuned_rep.dimness > (int)DIM_UB) tuned_rep.dimness = (int)DIM_UB;
	if (tuned_rep.dimness < (int)DIM_LB) tuned_rep.dimness = (int)DIM_LB;

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

OptiAlgo::ProblemRepresentation OptiAlgo::TabuSearch::search(ProblemRepresentation problem, AudioProps audio_props)
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
			temp_neighbour = problem.tune(problem.representation);
			neighbours.push_back(make_pair(temp_neighbour, (problem.objective_function(temp_neighbour, audio_props) - problem.objective_function(problem.representation, audio_props))));
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

map<string, double> OptiAlgo::execute_algorithm(TabuSearch algo, int n_iterations)
{
	double sum;
	double sum_time;
	clock_t begin, end;
	list<double> values;
	ProblemRepresentation problem;
	double mean, variance;
	audio_props = AudioProps();

	// Execute search multiple times, compute statistics
	sum = 0;
	sum_time = 0.0;
	for (int i = 0; i < n_iterations; i++)
	{
		problem = ProblemRepresentation(audio_props);
		begin = clock();
		problem = algo.search(problem, audio_props);
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

void OptiAlgo::test_lights()
{
	LightsInfo lights_config;
	int strobe = 0;
	char * err_str;

	while (!terminate)
	{
		try
		{
			lights_config.red_intensity = 255;
			lights_config.blue_intensity = 0;
			lights_config.green_intensity = 0;
			lights_config.white_intensity = 100;
			lights_config.strobing_speed = strobe;
			lights_config.dimness = 255;
			DMXOutput::updateLightsOutputQueue(lights_config);
			Sleep(200);
			lights_config.red_intensity = 0;
			lights_config.blue_intensity = 0;
			lights_config.green_intensity = 255;
			lights_config.white_intensity = 100;
			lights_config.strobing_speed = strobe;
			lights_config.dimness = 255;
			DMXOutput::updateLightsOutputQueue(lights_config);
			Sleep(200);
			lights_config.red_intensity = 0;
			lights_config.blue_intensity = 255;
			lights_config.green_intensity = 0;
			lights_config.white_intensity = 100;
			lights_config.strobing_speed = strobe;
			lights_config.dimness = 255;
			DMXOutput::updateLightsOutputQueue(lights_config);
			//strobe = (strobe + 100) % 200;
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
	//unsigned int nudges = 0, silences = 0;
	//bool listen_for_silence = false;

	unsigned int tenure = 5, n_iterations = 10;
	TabuSearch algorithm = TabuSearch(tenure, n_iterations);
	ProblemRepresentation solution;
	AudioInfo audio_sample, smoothed_input;
	audio_props = AudioProps();
	LightsInfo lights_config;

	double avg_freq=0.0, avg_tempo=0.0, avg_loud=0.0, avg_max_loud=0.0;
	double cur_freq, cur_tempo, cur_loud;
	bool got_freq, got_tempo, got_loud;
	double tempo_hist[HISTORY_BUF_SIZE];
	double loud_hist[HISTORY_BUF_SIZE];
	double max_loud_hist[MAX_LOUD_HIST_BUF_SIZE];
	int tempo_hist_it=0, loud_hist_it=0, max_loud_hist_it=0;

	stringstream out_str;
	char * err_str;

	// NT: add stop on silence
	while (!terminate) //&& (!listen_for_silence || silences < SILENCES_TO_STOP))
	{
		try {
			// Wait for audio input samples
			while (audio_buffer.empty()) {
				if (terminate) { return; }
			}

			// Grab sample
			audio_sample = audio_buffer.front();
			got_freq = audio_sample.get_frequency(cur_freq);
			got_tempo = audio_sample.get_tempo(cur_tempo);
			got_loud = audio_sample.get_loudness(cur_loud);
			audio_buffer.pop();

			// Update averages & history
			if (got_tempo)
			{
				tempo_hist[tempo_hist_it] = cur_tempo;
				tempo_hist_it = (tempo_hist_it + 1) % HISTORY_BUF_SIZE;
				avg_tempo = 0.0;
			}
			if (got_loud)
			{
				loud_hist[loud_hist_it] = cur_loud;
				loud_hist_it = (loud_hist_it + 1) % HISTORY_BUF_SIZE;
				avg_loud = 0.0;
			}
			for (int i = 0; i < HISTORY_BUF_SIZE; i++)
			{
				if (got_tempo) avg_tempo += tempo_hist[i];
				if (got_loud) avg_loud += loud_hist[i];
			}
			if (got_tempo) avg_tempo /= HISTORY_BUF_SIZE;
			if (got_loud) avg_loud /= HISTORY_BUF_SIZE;

			if (got_loud && avg_loud > 0.0 && abs(cur_loud - avg_loud) >= BEAT_THRESH)
			{
				max_loud_hist[max_loud_hist_it] = cur_loud;
				max_loud_hist_it = (max_loud_hist_it + 1) % MAX_LOUD_HIST_BUF_SIZE;
			}

			// TODO: detect and accommodate change

			// Check for silence
			//if (listen_for_silence)
			//{
			//	if (audio_sample.get_loudness(current_loudness) && current_loudness <= SILENCE_THRESH) silences++;
			//	else silences -= silences > 2 ? 2 : 0;
			//}


			// Adjust property weights
			smoothed_input = AudioInfo(NULL, &avg_loud, &avg_tempo);
			audio_props.adjust_weights(smoothed_input, avg_max_loud);

			// Use properties to find varying, near-optimal lights configuration
			solution = algorithm.search(solution, audio_props);

			// Make output consistent
			lights_config = solution.representation;
			// TODO: Fix this
			//temp_history = sample_history;
			//nudges = 0;
			//while (!temp_history.empty())
			//{
			//	hist_sol = temp_history.front();
			//	temp_history.pop();
			//	if (!temp_history.empty())
			//	{
			//		if (hist_sol.representation.differences(temp_history.front().representation) >= DIFFS_FOR_CHANGE)
			//		{
			//			break;
			//		}
			//		else
			//		{
			//			nudges++;
			//		}
			//	}
			//	else
			//	{
			//		nudges = 0;
			//		break;
			//	}
			//}
			//lights_config = (nudges >= NUDGES_TO_CHANGE) ? solution.representation : lights_config; // wrong logic :/

			// Send solution to output controller
			lights_config.strobing_speed = 0; // TODO: remove this
			out_str << "Sending to output [";
			out_str << lights_config.red_intensity << ",";
			out_str << lights_config.blue_intensity << ",";
			out_str << lights_config.green_intensity << ",";
			out_str << lights_config.white_intensity << ",";
			out_str << lights_config.dimness << ",";
			out_str << lights_config.strobing_speed << "]\n";
			Helpers::print_debug(out_str.str().c_str());
			out_str.str("");	//Clear string
			DMXOutput::updateLightsOutputQueue(lights_config);

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