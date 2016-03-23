#ifndef CONSTRUCTS_INCLUDE
#define CONSTRUCTS_INCLUDE

#include <Constants.h>
#include <deque>

using namespace std;

enum OutParams { r, g, b };

class AudioInfo
{
	double * _frequency;
	double * _loudness;
	double * _tempo;

public:
	AudioInfo(bool random = false);
	AudioInfo(double * freq, double * loudness, double * tempo);
	~AudioInfo();

	void randomize_info();

	bool operator == (const AudioInfo& b) const;
	AudioInfo differences(AudioInfo b);

	bool get_frequency(double & freq) const;
	void set_frequency(double freq);
	bool get_loudness(double & loud) const;
	void set_loudness(double freq);
	bool get_tempo(double & tempo) const;
	void set_tempo(double freq);
	void clear_tempo();
};


class LightsInfo
{
public:
	int red_intensity, blue_intensity, green_intensity, white_intensity;
	int strobing_speed, dimness;

	LightsInfo();
	LightsInfo(bool centered);
	void Reset();

	bool operator == (const LightsInfo& b) const;

	unsigned char * convert_to_output(unsigned char lightsOutput[]);
	static LightsInfo average_and_smooth(deque<LightsInfo> outputs);
};

class SectionInfo
{
public:
	SECTION section;
	bool should_strobe;

	SectionInfo();
	SectionInfo(SECTION section, bool should_strobe);
};

#endif