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

map<string, double> OptiAlgo::AudioProps::new_modifiers_set(const double * mods)
{
	map<string, double> modifiers;
	modifiers.insert(make_pair("r", mods[0]));
	modifiers.insert(make_pair("R", mods[1]));
	modifiers.insert(make_pair("g", mods[2]));
	modifiers.insert(make_pair("G", mods[3]));
	modifiers.insert(make_pair("b", mods[4]));
	modifiers.insert(make_pair("B", mods[5]));
	modifiers.insert(make_pair("w", mods[6]));
	modifiers.insert(make_pair("W", mods[7]));
	modifiers.insert(make_pair("dim", mods[8]));
	modifiers.insert(make_pair("DIM", mods[9]));
	return modifiers;
}

void OptiAlgo::AudioProps::adjust_weights(AudioInfo input, double max_loud)
{
	double freq, tempo, loud;
	input.get_frequency(freq);
	input.get_tempo(tempo);
	input.get_loudness(loud);

	// NT: set freq conc weight
	freq_conc_add_and_avg(0.0);
	overall_tempo_add_and_avg( 0.5 * (loud / LOUD_UB) * (1 + tempo / TEMPO_UB) ); // NT: add freq to this weight
	beatiness_add_and_avg( ((loud / max_loud) <= 0.4) ? (1 - 0.8*(loud / max_loud)) : (0.8 - 2*0.8*((loud / max_loud) - 0.5)) ); // NT: add freq to this weight
	overall_tempo_add_and_avg( (tempo / TEMPO_UB) );
}

double OptiAlgo::AudioProps::freq_conc_add_and_avg(double val)
{
	double avg = 0.0;
	freq_conc_hist.push_front(val);
	if (freq_conc_hist.size() > WEIGHTS_HIST_BUF_SIZE) freq_conc_hist.pop_back();
	for each (double val in freq_conc_hist)
		avg += val;
	avg /= (double)freq_conc_hist.size();
	freq_conc.second = avg;
	return avg;
}
double OptiAlgo::AudioProps::overall_tempo_add_and_avg(double val)
{
	double avg = 0.0;
	overall_tempo_hist.push_front(val);
	if (overall_tempo_hist.size() > WEIGHTS_HIST_BUF_SIZE) overall_tempo_hist.pop_back();
	for each (double val in overall_tempo_hist)
		avg += val;
	avg /= (double)overall_tempo_hist.size();
	overall_tempo.second = avg;
	return avg;
}
double OptiAlgo::AudioProps::beatiness_add_and_avg(double val)
{
	double avg = 0.0;
	beatiness_hist.push_front(val);
	if (beatiness_hist.size() > WEIGHTS_HIST_BUF_SIZE) beatiness_hist.pop_back();
	for each (double val in beatiness_hist)
		avg += val;
	avg /= (double)beatiness_hist.size();
	beatiness.second = avg;
	return avg;
}
double OptiAlgo::AudioProps::overall_intens_add_and_avg(double val)
{
	double avg = 0.0;
	overall_intens_hist.push_front(val);
	if (overall_intens_hist.size() > WEIGHTS_HIST_BUF_SIZE) overall_intens_hist.pop_back();
	for each (double val in overall_intens_hist)
		avg += val;
	avg /= (double)overall_intens_hist.size();
	overall_intens.second = avg;
	return avg;
}

#pragma endregion


#pragma region ProblemRepresentation

OptiAlgo::ProblemRepresentation::ProblemRepresentation()
{
	representation = LightsInfo();
	rep_value = 0;
}
OptiAlgo::ProblemRepresentation::ProblemRepresentation(AudioProps properties, bool centered)
{
	// Start with a random candidate solution
	representation = LightsInfo(centered);
	rep_value = objective_function(representation, properties);
}

double OptiAlgo::ProblemRepresentation::objective_function(LightsInfo cand_sol, AudioProps props)
{
	// Applies objective function to current solution
	double obf_freq_conc, obf_overall_tempo, obf_beatiness, obf_overall_intens;
	double weight, r, R, g, G, b, B, w, W, dim, DIM;

	obf_freq_conc = 0.0; // NT: add freq conc portion of objective function

	//weight = props.overall_tempo.second;
	//r = props.overall_tempo.first["r"];
	//R = props.overall_tempo.first["R"];
	//g = props.overall_tempo.first["g"];
	//G = props.overall_tempo.first["G"];
	//b = props.overall_tempo.first["b"];
	//B = props.overall_tempo.first["B"];
	//w = props.overall_tempo.first["w"];
	//W = props.overall_tempo.first["W"];
	//obf_overall_tempo = 1 / (abs(B + (b - B)*weight - (double)cand_sol.blue_intensity) + 1) +
	//	1 / (abs(r + (R - r)*weight - (double)cand_sol.red_intensity) + 1) +
	//	1 / (abs(g + (G - g)*weight - (double)cand_sol.green_intensity) + 1) +
	//	1 / (abs(w + (W - w)*weight - (double)cand_sol.white_intensity) + 1); // NT: add strobing stuff
	obf_overall_tempo = 0.0;

	weight = props.beatiness.second;
	r = props.beatiness.first["r"];
	R = props.beatiness.first["R"];
	g = props.beatiness.first["g"];
	G = props.beatiness.first["G"];
	b = props.beatiness.first["b"];
	B = props.beatiness.first["B"];
	w = props.beatiness.first["w"];
	W = props.beatiness.first["W"];
	dim = props.beatiness.first["dim"];
	DIM = props.beatiness.first["DIM"];
	obf_beatiness = 1 / (abs(G + (g - G)*weight - (double)cand_sol.green_intensity) + 1) +
		1 / (abs(r + (R - r)*weight - (double)cand_sol.red_intensity) + 1) +
		1 / (abs(B + (b - B)*weight - (double)cand_sol.blue_intensity) + 1) +
		1 / (abs(W + (w - W)*weight - (double)cand_sol.white_intensity) + 1) +
		1 / (abs(dim + (DIM - dim)*weight - (double)cand_sol.dimness) + 1); // NT: add strobing stuff
	obf_beatiness = 0.0;

	weight = props.overall_intens.second;
	r = props.overall_intens.first["r"];
	R = props.overall_intens.first["R"];
	g = props.overall_intens.first["g"];
	G = props.overall_intens.first["G"];
	b = props.overall_intens.first["b"];
	B = props.overall_intens.first["B"];
	w = props.overall_intens.first["w"];
	W = props.overall_intens.first["W"];
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
		problem = ProblemRepresentation(audio_props, false);
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
			Sleep(200);
			lights_config.red_intensity = 0;
			lights_config.blue_intensity = 0;
			lights_config.green_intensity = 255;
			lights_config.white_intensity = w;
			lights_config.strobing_speed = strobe;
			lights_config.dimness = 255;
			DMXOutput::updateLightsOutputQueue(lights_config);
			Sleep(200);
			lights_config.red_intensity = 0;
			lights_config.blue_intensity = 255;
			lights_config.green_intensity = 0;
			lights_config.white_intensity = w;
			lights_config.strobing_speed = strobe;
			lights_config.dimness = 255;
			DMXOutput::updateLightsOutputQueue(lights_config);
			//strobe = (strobe + 100) % 200;
			w += 10;
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

	unsigned int tenure = TENURE_START, n_iterations = 10;
	TabuSearch algorithm = TabuSearch(tenure, n_iterations);
	ProblemRepresentation cur_sol;
	AudioInfo audio_sample, smoothed_input;
	audio_props = AudioProps();
	int tenure_cooldown = 0;

	double avg_freq=0.0, avg_tempo=0.0, avg_loud=0.0, avg_max_loud=0.0;
	double cur_freq, cur_tempo, cur_loud;
	bool got_freq, got_tempo, got_loud;

	deque<double> tempo_hist = deque<double>();
	deque<double> loud_hist = deque<double>();
	deque<double> max_loud_hist = deque<double>();

	deque<LightsInfo> out_hist = deque<LightsInfo>();
	LightsInfo avg_out = LightsInfo(true);

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

			out_str.str("");

			// Grab sample
			audio_sample = audio_buffer.front();
			got_freq = audio_sample.get_frequency(cur_freq);
			got_tempo = audio_sample.get_tempo(cur_tempo);
			got_loud = audio_sample.get_loudness(cur_loud);
			audio_buffer.pop();

			out_str << "NULL" << ",";
			if (got_tempo) out_str << cur_tempo << ","; else out_str << "NULL" << ",";
			if (got_loud) out_str << cur_loud; else out_str << "NULL";

			// NT: Check for silence
			//if (listen_for_silence)
			//{
			//	if (audio_sample.get_loudness(current_loudness) && current_loudness <= SILENCE_THRESH) silences++;
			//	else silences -= silences > 2 ? 2 : 0;
			//}

			// Update averages & history
			if (got_tempo)
			{
				tempo_hist.push_front(cur_tempo);
				if (tempo_hist.size() > HISTORY_BUF_SIZE) tempo_hist.pop_back();
				avg_tempo = 0.0;
				for each (double val in tempo_hist)
					avg_tempo += val;
				avg_tempo /= tempo_hist.size();
			}
			if (got_loud)
			{
				if (avg_loud > 0.0 && cur_loud - avg_loud >= BEAT_THRESH)
				{
					max_loud_hist.push_front(cur_loud);
					if (max_loud_hist.size() > MAX_LOUD_HIST_BUF_SIZE) max_loud_hist.pop_back();
					avg_max_loud = 0.0;
					for each (double val in max_loud_hist)
						avg_max_loud += val;
					avg_max_loud /= max_loud_hist.size();
				}
				else
				{
					loud_hist.push_front(cur_loud);
					if (loud_hist.size() > HISTORY_BUF_SIZE) loud_hist.pop_back();
					avg_loud = 0.0;
					for each (double val in loud_hist)
						avg_loud += val;
					avg_loud /= loud_hist.size();
				}
			}
			if (avg_max_loud < avg_loud) avg_max_loud = avg_loud;

			out_str << " => ";
			out_str << avg_freq << "," << avg_tempo << "," << avg_loud << "," << avg_max_loud;

			// TODO: detect and accommodate change

			// Adjust property weights
			smoothed_input = AudioInfo(NULL, &avg_loud, &avg_tempo);
			audio_props.adjust_weights(smoothed_input, avg_max_loud);

			audio_props.freq_conc.second = 0.0;
			audio_props.beatiness.second = 0.0;
			audio_props.overall_tempo.second = 0.0;
			audio_props.overall_intens.second = 1.0;

			out_str << " => ";
			out_str << audio_props.beatiness.second << ","
					<< audio_props.overall_tempo.second << ","
					<< audio_props.overall_intens.second;

			// Use properties to find varying, near-optimal lights configuration
			cur_sol.representation = avg_out;
			cur_sol = algorithm.search(cur_sol, audio_props);

			out_str << " => ";
			out_str << "[" << cur_sol.representation.red_intensity << ","
				<< cur_sol.representation.green_intensity << ","
				<< cur_sol.representation.blue_intensity << ","
				<< cur_sol.representation.white_intensity << ","
				<< cur_sol.representation.dimness << ","
				<< cur_sol.representation.strobing_speed << "]";

			// Average output
			out_hist.push_front(cur_sol.representation);
			if (out_hist.size() > OUT_HIST_BUF_SIZE) out_hist.pop_back();
			avg_out = LightsInfo::average_and_smooth(out_hist);

			// Send solution to output controller
			avg_out.strobing_speed = 0; // TODO: remove this

			out_str << " => ";
			out_str << "[" << avg_out.red_intensity << "," << avg_out.green_intensity << "," << avg_out.blue_intensity << ","
				<< avg_out.white_intensity << "," << avg_out.dimness << "," << avg_out.strobing_speed << "]\n";
			Helpers::print_debug(out_str.str().c_str());

			DMXOutput::updateLightsOutputQueue(avg_out);

			// Cool down tenure
			tenure_cooldown = (tenure_cooldown + 1) % TENURE_COOLDOWN;
			if (tenure > 0 && tenure_cooldown == 0) tenure--;

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