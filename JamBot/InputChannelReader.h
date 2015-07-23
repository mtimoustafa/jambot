#include "stdafx.h"
#include "portaudio.h"
#include <queue>

#define SAMPLE_RATE  (22050)
#define FRAMES_PER_BUFFER (4410)	//200ms of audio per buffer
#define NUM_CHANNELS    (1)
#define IS_STEREO		(0)
#define DITHER_FLAG     (0)
#define SAMPLE_SILENCE  (0)
#define PRINTF_S_FORMAT "%d"

typedef struct
{
	float*					recordedSamples;
	std::queue<float*>		recordedBuffer;
	int						bufferedSamples;
}
paData;

class InputChannelReader
{
public:
	InputChannelReader();

	void start();

	void stop();

private:
	bool stopStream;

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