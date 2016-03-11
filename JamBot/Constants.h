#ifndef CONSTANTS_INCLUDE
#define CONSTANTS_INCLUDE

// InputChannelReader Parameters
const int SAMPLE_RATE = 22050;
const int FRAMES_PER_BUFFER = 4410;	//200ms of audio per buffer
const int NUM_CHANNELS = 2;
const int NUM_SAMPLES = FRAMES_PER_BUFFER * NUM_CHANNELS * 2;
const int FFT_SIZE = 4096;
const int OUTPUT_SIZE = (int)(FFT_SIZE / 2) + 1;
const int HIGHEST_PITCH = 7000;
const int LOWEST_PITCH = 80;
const int MAX_I = HIGHEST_PITCH * OUTPUT_SIZE / SAMPLE_RATE;
const int MIN_I = LOWEST_PITCH * OUTPUT_SIZE / SAMPLE_RATE;
const int REC_CHANNEL = 1;
const int HPS_LENGTH = MAX_I - MIN_I;

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
const unsigned int HISTORY_BUF_SIZE = 10;
const unsigned int MAX_LOUD_HIST_BUF_SIZE = 4;
const unsigned int WEIGHTS_HIST_BUF_SIZE = 1;
const unsigned int OUT_HIST_BUF_SIZE = 20;
const unsigned int NUDGES_TO_CHANGE = 3;
const unsigned int SILENCES_TO_STOP = 10 * 5;
const unsigned int DIFFS_FOR_CHANGE = 4;

const int TOO_SMALL_SMOOTH_THRESH = 20;
const int CHANGE_TO_MAX_LOUD_THRESH = 500.0;

const int TENURE_START = 6;
const int TENURE_COOLDOWN = 2;

const double BEAT_THRESH = 1000.0;
const double SILENCE_THRESH = 400.0;

const double MODIFIERS[10] = { 0.0, 255.0, 0.0, 255.0, 0.0, 255.0, 0.0, 0.0, 0.0, 255.0 };

const double TUNE_UB = 10.0;
const double TUNE_LB = -10.0;

const enum SECTION {chorus, verse};
#pragma endregion

#endif