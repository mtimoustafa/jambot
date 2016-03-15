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
const int NUM_PEAKS = 3;
const unsigned int AUDIO_BUF_SIZE = 256;

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

// TODO: tweak these parameters
const unsigned int IN_HIST_BUF_SIZE = 5;
const unsigned int OUT_HIST_BUF_SIZE = 20;

const double BEATINESS_THRESH = 1000.0; // Remove this and use class
const double SILENCE_THRESH = 100.0; //TODO: tweak this

const enum SECTION { chorus, verse };

#endif