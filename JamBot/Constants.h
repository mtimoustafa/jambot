#ifndef CONSTANTS_INCLUDE
#define CONSTANTS_INCLUDE

// AudioInfo parameter bounds
const double FREQ_UB = 20000.0;
const double FREQ_LB = 0.0;
const double LOUD_UB = 18000.0;
const double LOUD_LB = 0.0;
const double TEMPO_UB = 200.0;
const double TEMPO_LB = 60.0;

// AudioInfo thresholds
const double AI_EQUAL_THRESH = 2.0;

//LightsInfo thresholds
const double LI_EQUAL_THRESH = 2.0;

//Output parameter bounds
const double R_UB = 255.0;
const double R_LB = 0.0;
const double B_UB = 255.0;
const double B_LB = 0.0;
const double G_UB = 255.0;
const double G_LB = 0.0;
const double W_UB = 255.0;
const double W_LB = 0.0;
const double DIM_UB = 255.0;
const double DIM_LB = 0.0;

#pragma region Optimization Algorithm parameters
const unsigned int AUDIO_BUF_SIZE = 256;
const unsigned int HISTORY_BUF_SIZE = 5;
const unsigned int MAX_LOUD_HIST_BUF_SIZE = 5;
const unsigned int NUDGES_TO_CHANGE = 3;
const unsigned int SILENCES_TO_STOP = 10 * 5;
const unsigned int DIFFS_FOR_CHANGE = 4;

const double FREQ_SMOOTH_THRESH = 50.0;
const double LOUD_SMOOTH_THRESH = 50.0;
const double TEMPO_SMOOTH_THRESH = 50.0;

const double BEAT_THRESH = 1000.0;
const double SILENCE_THRESH = 10.0;

const double FREQ_MODS[14] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
const double OV_TEMPO_MODS[14] = { 0.0, 255.0, 0.0, 255.0, 255.0, 100.0, 0.0, 255.0, 0.0, 0.0, 0.0, 255.0, 180.0, 0.0 };
const double BEATI_MODS[14] = { 255.0, 100.0, 0.0, 255.0, 255.0, 100.0, 0.0, 255.0, 0.0, 255.0, 0.0, 255.0, 0.0, 255.0 };
const double OV_INT_MODS[14] = { 0.0, 0.0, 0.0, 255.0, 0.0, 0.0, 0.0, 200.0, 0.0, 200.0, 0.0, 0.0, 0.0, 0.0 };

const double TUNE_UB = 10.0;
const double TUNE_LB = -10.0;
#pragma endregion

#endif