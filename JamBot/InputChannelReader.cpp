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
#include "Constants.h"
#include "Constructs.h"
#include "write_wav.cpp"
#include "MiniBpm.cpp"
#include <string>
#include <queue>

MiniBPM tempo = MiniBPM((float)SAMPLE_RATE);
AudioInfo audioSamples = AudioInfo();

InputChannelReader::InputChannelReader() 
{
	stopStream = false;
};

/* This routine will be called by the PortAudio engine when audio is needed.
** It may be called at interrupt level on some machines so don't do anything
** that could mess up the system like calling malloc() or free().
*/
int InputChannelReader::recordCallback(const void *inputBuffer, void *outputBuffer,
	unsigned long framesPerBuffer,
	const PaStreamCallbackTimeInfo* timeInfo,
	PaStreamCallbackFlags statusFlags,
	void *userData)
{
	paData *data = (paData *)userData;
	const short *rptr = (const short*)inputBuffer;
	float *wptr = data->recordedSamples;
	int numSamples = FRAMES_PER_BUFFER * NUM_CHANNELS * 2;

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
				*wptr++ = (float)SAMPLE_SILENCE;  //left
				*wptr++ = (float)SAMPLE_SILENCE;  //right
			}
		}
	}
	else
	{
		for (int i = 0; i < FRAMES_PER_BUFFER; i += NUM_CHANNELS)
		{
			for (int j = 0; j < NUM_CHANNELS; j++)
			{
				if (IS_STEREO)	//left
				{
					*wptr++ = (float)*rptr++;
				}
				else
				{
					*wptr++ = (float)*rptr;
				}
				*wptr++ = (float)*rptr++;	//right
			}
		}
	}

	// Save Buffer
	data->recordedBuffer.push(data->recordedSamples);
	data->recordedSamples = new float[numSamples];	//Reaollocate recorded samples

	return 0;
}

// This routine will be called whenever a Buffer has finished recording
void InputChannelReader::analyseBuffer(paData *data)
{
	float val = 0.0;
	double average = 0.0;
	int numSamples = FRAMES_PER_BUFFER * NUM_CHANNELS * 2;

	// Measure average peak amplitude
	for (int i = 0; i < numSamples; i++)
	{
		val = data->recordedBuffer.back()[i];
		if (val < 0) val = -val; /* ABS */
		average += val;
	}
	average = average / (double)numSamples;
	audioSamples.set_loudness(average);

	Helpers::print_debug(("Average sample loudness (dB): " + to_string(average) + "\n").c_str());

	// Measure average tempo every 1.5s
	tempo.process(const_cast<float*>(data->recordedBuffer.back()), numSamples);

	if ((data->recordedBuffer.size() % 15) == 0)
	{
		audioSamples.set_tempo(tempo.estimateTempo());
		Helpers::print_debug(("Average sample tempo (bpm): " + to_string(tempo.estimateTempo()) + "\n\n").c_str());
		tempo.reset();
	}
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
	int                 i, j = 0;
	int                 numSamples;
	int					totalSamples;
	int                 numBytes;
	float*				streamBuffers;

	tempo.setBPMRange(TEMPO_LB, TEMPO_UB);	//Set up tempo ranges

	numSamples = FRAMES_PER_BUFFER * NUM_CHANNELS * 2;
	numBytes = numSamples * sizeof(float);
	data.recordedSamples = (float *)malloc(numBytes); //From now on, recordedSamples is initialised. 
	data.bufferedSamples = 0;
	if (data.recordedSamples == NULL)
	{
		Helpers::print_debug("Could not allocate record array.\n");
		goto done;
	}
	for (i = 0; i<numSamples; i++) data.recordedSamples[i] = 0.0;

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

	/* Record some audio. -------------------------------------------- */
	err = Pa_OpenStream(
		&stream,
		&inputParameters,
		NULL,                  /* &outputParameters, */
		SAMPLE_RATE,
		FRAMES_PER_BUFFER,
		paClipOff,      /* we won't output out of range samples so don't bother clipping them */
		recordCallback,
		&data);
	if (err != paNoError) goto done;

	err = Pa_StartStream(stream);
	if (err != paNoError) goto done;
	Helpers::print_debug("\n=== Now recording! Please speak into the microphone. ===\n");

	while (!stopStream)
	{
		// Analyse Buffer when new Buffer available
		if (data.bufferedSamples != data.recordedBuffer.size())
		{
			analyseBuffer(&data);
			data.bufferedSamples++;
		}
	}

	err = Pa_CloseStream(stream);
	if (err != paNoError) goto done;

	totalSamples = numSamples * data.recordedBuffer.size();
	streamBuffers = (float *)malloc(totalSamples*sizeof(float));

	// Load all Buffer samples
	while (!data.recordedBuffer.empty())
	{
		for (i = 0; i < numSamples; i++)
		{
			streamBuffers[j] = data.recordedBuffer.front()[i];
			j++;
		}
		data.recordedBuffer.pop();
	}

	// Write recorded data to a file. 
	WAV_Writer *writer = new WAV_Writer();
	long openFile, writeFile;
	openFile = Audio_WAV_OpenWriter(writer, "Output.wav", SAMPLE_RATE, 2);
	if (openFile < 0)
	{
		Helpers::print_debug("Could not open file.\n");
	}
	else
	{
		writeFile = Audio_WAV_WriteShorts(writer, streamBuffers, totalSamples);
		if (writeFile < 0)
		{
			Helpers::print_debug("Could not write file.\n");
		}
		else
		{
			Helpers::print_debug(("Wrote " + to_string(writer->dataSize) + " bytes of data to 'Output.wav'.\n").c_str());
		}
	}
	Audio_WAV_CloseWriter(writer);

	free(streamBuffers);

done:
	Pa_Terminate();
	if (data.recordedSamples)       /* Sure it is NULL or valid. */
		free(data.recordedSamples);
	if (err != paNoError)
	{
		Helpers::print_debug("ERROR: Terminated InputChannelReader module");
		err = 1;          /* Always return 0 or 1, but no other return codes. */
	}

	return err;
}