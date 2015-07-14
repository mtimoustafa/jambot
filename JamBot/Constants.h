#ifndef CONSTANTS_INCLUDE
#define CONSTANTS_INCLUDE

// AudioInfo parameter bounds
const double FREQ_UB = 20000.0;
const double FREQ_LB = 20.0;
const double LOUD_UB = 150.0;
const double LOUD_LB = 0.0;
const double TEMPO_UB = 500.0;
const double TEMPO_LB = 10.0;

#pragma region Optimization Algorithm parameters

const double FREQ_MODS [8] = { 0.8, 0.0, 0.2, 100.0, 127.0, 0.0, 0.0, 0.0 };
const double PACE_MODS[8] = { 0.0, 0.5, 0.0, 0.0, 220, 0.0, 0.0, 0.0 };
const double BEATI_MODS[8] = { 0.5, 0.0, 0.5, 0.0, 250, 0.0, 127.0, 127.0 };
const double OV_INT_MODS[8] = { 0.5, 0.5, 0.5, 127.0, 200.0, 0.0, 0.0, 0.0 };

const double TUNE_UB = 10.0;
const double TUNE_LB = -10.0;
#pragma endregion

#endif