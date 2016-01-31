/*
* $Id: paex_record.c 1752 2011-09-08 03:21:55Z philburk $
*
* This program uses the PortAudio Portable Audio Library.
* For more information see: http://www.portaudio.com
* Copyright (c) 1999-2000 Ross Bencina and Phil Burk
*
* Permission is hereby granted, free of charge, to any person obtaining
* a copy of this software and associated documentation files
* (the "Software"), to deal in the Software without restriction,
* including without limitation the rights to use, copy, modify, merge,
* publish, distribute, sublicense, and/or sell copies of the Software,
* and to permit persons to whom the Software is furnished to do so,
* subject to the following conditions:
*
* The above copyright notice and this permission notice shall be
* included in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR
* ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
* CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
* WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

/*
* The text above constitutes the entire PortAudio license; however,
* the PortAudio community also makes the following non-binding requests:
*
* Any person wishing to distribute modifications to the Software is
* requested to send the modifications to the original developer so that
* they can be incorporated into the canonical version. It is also
* requested that these non-binding requests be included along with the
* license above.
*/

#include "stdafx.h"
#include "Helpers.h"
#include "InputChannelReader.h"
#include "portaudio.h"
#include "OptiAlgo.h"
#include "WavManipulation.h"
#include "Constants.h"
#include "Constructs.h"
#include "soundfile.h"
#include "write_wav.cpp"
#include "MiniBpm.cpp"
#include "fftw3.h"
#include <string>
#include <vector>
#include <math.h>

float *in;
fftwf_complex *out;
fftwf_plan frequencyPlan;
MiniBPM tempo = MiniBPM((float)SAMPLE_RATE);
AudioInfo audioSamples = AudioInfo();
SoundHeader header = SoundHeader();
SoundHeader outHeader = SoundHeader();

InputChannelReader::InputChannelReader() 
{
	stopStream = false;
};

// This routine will be called by the PortAudio engine when audio is needed.
// It may be called at interrupt level on some machines so don't do anything
// that could mess up the system like calling malloc() or free().

int InputChannelReader::recordCallback(const void *inputBuffer, void *outputBuffer,
	unsigned long framesPerBuffer,
	const PaStreamCallbackTimeInfo* timeInfo,
	PaStreamCallbackFlags statusFlags,
	void *userData)
{
	std::vector<float> recordedSamples(NUM_SAMPLES, 0.0);
	std::vector<float>::iterator it = recordedSamples.begin();
	paData *data = (paData *)userData;
	const short *rptr = (const short*)inputBuffer;

	// Prevent unused variable warnings.
	(void)outputBuffer;
	(void)timeInfo;
	(void)statusFlags;
	(void)userData;

	if (inputBuffer == NULL)
	{
		for (int i = 0; i<FRAMES_PER_BUFFER; i += NUM_CHANNELS)
		{
			for (int j = 0; j < NUM_CHANNELS; j++)
			{
				// Skip two channels
				it += 2;
			}
		}
	}
	else
	{
		for (int i = 0; i < FRAMES_PER_BUFFER; i += NUM_CHANNELS)
		{
			for (int j = 0; j < NUM_CHANNELS; j++)
			{
				if (IS_STEREO)	//Left
				{
					*it++ = (float)*rptr++;
				}
				else
				{
					*it++ = (float)*rptr;
				}
				*it++ = (float)*rptr++;	//Right
			}
		}
	}

	// Save Buffer
	data->recordedSamples = recordedSamples;

	return 0;
}

// This routine will be called whenever the Hanning function is needed
float InputChannelReader::hannFunction(int n)
{
	double inner = (2 * M_PI * n) / (FFT_SIZE - 1);
	return (float)(0.5 * (1.0 - cos(inner)));
}

// This routine will be called whenever a Buffer has finished recording
void InputChannelReader::analyseBuffer(paData *data)
{
	float val = 0.0;
	float magnitude;
	float maxDensity[NUM_PEAKS];
	float frequency[NUM_PEAKS];
	int maxIndex[NUM_PEAKS];
	double average = 0.0;
	short fileSamples;

	// Initialize maximum spectrum peaks to zeroes
	for (int i = 0; i < NUM_PEAKS; i++)
	{
		maxDensity[i] = 0.0;
	}

	// Measure average peak amplitude
	for (int i = 0; i < NUM_SAMPLES; i++)
	{
		// Use every-other sample
		if (i < FFT_SIZE && i % 2 == 0)
		{
			in[i] = data->recordedSamples[i] * hannFunction(i);
		}
		val = data->recordedSamples[i];
		if (val < 0) val = -val;
		average += val;
	}
	average = average / (double)NUM_SAMPLES;
	audioSamples.set_loudness(average);
	Helpers::print_debug(("Average sample loudness (dB): " + to_string(average) + "\n").c_str());

	// Get frequency of wave
	fftwf_execute(frequencyPlan);

	for (int i = 0; i < 1200; i++)	//Fix to get whole OUTPUT_SIZE range checked
	{
		magnitude = (float)sqrt(pow(out[i][0], 2) + pow(out[i][1], 2));

		for (int j = 0; j < NUM_PEAKS; j++)
		{
			if (magnitude > maxDensity[j])
			{
				maxDensity[j] = magnitude;
				maxIndex[j] = i;
				break;
			}
		}
	}

	for (int i = 0; i < NUM_PEAKS; i++)
	{
		frequency[i] = maxIndex[i] * SAMPLE_RATE / OUTPUT_SIZE;
		Helpers::print_debug(("Frequency peak " + to_string(i + 1) + " (Hz): " + to_string(frequency[i]) + "\n").c_str());
	}
	audioSamples.set_frequency((float)frequency[0]);

	// Measure average tempo every 1.5s
	tempo.process(const_cast<float*>(&data->recordedSamples[0]), NUM_SAMPLES);
	if ((data->numBuffers % 15) == 0)
	{
		audioSamples.set_tempo(tempo.estimateTempo());
		Helpers::print_debug(("Average sample tempo (bpm): " + to_string(tempo.estimateTempo()) + "\n\n").c_str());
		tempo.reset();
	}

	OptiAlgo::receive_audio_input_sample(audioSamples);	//Send to OptiAlgo
	WavManipulation::inputData(data->recordedSamples);	//Send to WavManipulation

	data->recordedSamples.clear();	//Empty recordedSamples

	//TODO: Add file writing capabilities
	//TODO: Add Multiple channel support
	//TODO: Add frequency analyzer
}

void InputChannelReader::stop()
{
	stopStream = true;
}

void InputChannelReader::start()
{
	main();
}

int InputChannelReader::main(void)
{
	PaStreamParameters  inputParameters;
	PaStream*           stream;
	PaError             err = paNoError;
	paData				data;
	int                 k = 0;

	tempo.setBPMRange(TEMPO_LB, TEMPO_UB);	//Set up tempo ranges
	tempo.setBeatsPerBar(2);	//Set up 2 beats per bar

	data.numBuffers = 0;

	// Initialize FFTW plan
	in = (float*)fftwf_malloc(sizeof(float) * FFT_SIZE);
	out = (fftwf_complex*)fftwf_malloc(sizeof(fftwf_complex) * OUTPUT_SIZE);
	frequencyPlan = fftwf_plan_dft_r2c_1d(FFT_SIZE, in, out, FFTW_ESTIMATE);

	err = Pa_Initialize();
	if (err != paNoError) goto done;

	inputParameters.device = Pa_GetDefaultInputDevice();	//Default input device
	if (inputParameters.device == paNoDevice) {
		goto done;
	}
	inputParameters.channelCount = NUM_CHANNELS * (1 + IS_STEREO);
	inputParameters.sampleFormat = paInt16;
	inputParameters.suggestedLatency = Pa_GetDeviceInfo(inputParameters.device)->defaultLowInputLatency;
	inputParameters.hostApiSpecificStreamInfo = NULL;

	// Record audio
	err = Pa_OpenStream(
		&stream,
		&inputParameters,
		NULL,                
		SAMPLE_RATE,
		FRAMES_PER_BUFFER,
		paClipOff,      
		recordCallback,
		&data);
	if (err != paNoError) goto done;

	err = Pa_StartStream(stream);
	if (err != paNoError) goto done;
	Helpers::print_debug("\n============= Now recording! =============\n");

	while (!stopStream)
	{
		// Analyse Buffer when new Buffer available
		if (!data.recordedSamples.empty())
		{
			analyseBuffer(&data);
			data.numBuffers++;
		}
	}

	err = Pa_CloseStream(stream);
	if (err != paNoError) goto done;

done:
	Pa_Terminate();
	data.recordedSamples.clear();
	fftwf_destroy_plan(frequencyPlan);
	fftwf_free(in); 
	fftwf_free(out);

	if (err != paNoError)
	{
		Helpers::print_debug("ERROR: Terminated InputChannelReader module");
		err = 1;       
	}

	return err;
}