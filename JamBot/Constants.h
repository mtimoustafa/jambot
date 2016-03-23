#ifndef CONSTANTS_INCLUDE
#define CONSTANTS_INCLUDE

// InputChannelReader Parameters
const int SAMPLE_RATE = 22050;
const int FRAMES_PER_BUFFER = 4410;	//200ms of audio per buffer
const int FFT_SIZE = 4096;
const int OUTPUT_SIZE = (int)(FFT_SIZE / 2) + 1;
const int HIGHEST_PITCH = 7000;
const int LOWEST_PITCH = 80;
const int MAX_I = HIGHEST_PITCH * OUTPUT_SIZE / SAMPLE_RATE;
const int MIN_I = LOWEST_PITCH * OUTPUT_SIZE / SAMPLE_RATE;
const int REC_CHANNEL = 1;
const int HPS_LENGTH = MAX_I - MIN_I;
const int NUM_PEAKS = 3;
const unsigned int AUDIO_BUF_SIZE = 256;

// AudioInfo parameter bounds
const double FREQ_UB = 20000.0;
const double FREQ_LB = 0.0;
const double LOUD_UB = 18000.0;
const double LOUD_LB = 0.0;
const double TEMPO_UB = 150.0;
const double TEMPO_LB = 80.0;

// AudioInfo thresholds
const double AI_EQUAL_THRESH = 2.0;

//LightsInfo thresholds
const double LI_EQUAL_THRESH = 2.0;
const int OUT_PARAM_TOO_SMALL_THRESH = 20;

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

const unsigned int IN_HIST_BUF_SIZE = 10;
const unsigned int OUT_HIST_BUF_SIZE = 10;
const unsigned int LOUDNESS_MAX_DETECT_BUF_SIZE = 5;

const double MAX_LOUDNESS_THRESH = 500.0;
const double BEATINESS_LOUDNESS_THRESH = 700.0;
const double BEATINESS_TEMPO_THRESH = 0.5;
const int MAX_LOUD_INSERTION_COOLDOWN = 4;
const int STROBING_COOLDOWN = 7;
const double SILENCE_THRESH = 40.0;
const double FREQ_VALUES_DISTANCE_THRESH = 50.0;
const double FREQ_HARMONIC_DETECT_THRESH = 10.0;
const double FREQ_ANOMALY_DETECT_THRESH = 100.0;
const int FREQ_HARMONIC_NUDGES_TO_CHANGE = 0; // TODO: do we need this?
const int FREQ_ANOMALY_NUDGES_TO_CHANGE = 0; // TODO: do we need this?
const double TEMPO_CLOSENESS_THRESH = 10.0;
const int TEMPO_NUDGES_TO_CHANGE = 3;
const double OUTPUT_TOO_LOW_THRESH = 10.0;
const int OUTPUT_ANOMALY_DETECT_THRESH = 50;
const int OUTPUT_NUDGES_TO_CHANGE = 5;

const int NUDGES_TO_SILENCE = 6;

const enum SECTION { none, chorus, verse };

#endif