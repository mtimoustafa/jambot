#include "stdafx.h"
#include "portaudio.h"
#include "Constants.h"
#include "soundfile.h"
#include <vector>

#define IS_STEREO		(1)

struct paData
{
	int					numBuffers;
	std::vector<float>	recordedSamples;
};

class InputChannelReader
{
public:
	InputChannelReader();

	void start();

	void stop();

private:
	bool stopStream;
	SoundHeader outHeader;

	// This routine will be called by the PortAudio engine when audio is needed.
	// It may be called at interrupt level on some machines so don't do anything
	// that could mess up the system like calling malloc() or free().
	static int recordCallback(const void *inputBuffer, void *outputBuffer,
		unsigned long framesPerBuffer,
		const PaStreamCallbackTimeInfo* timeInfo,
		PaStreamCallbackFlags statusFlags,
		void *userData);

	// This routine will be called whenever a Buffer has finished recording
	static void analyseBuffer(paData *data);

	int main(void);
};