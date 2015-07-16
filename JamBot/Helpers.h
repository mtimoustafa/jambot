#ifndef HELPERS_INCLUDE
#define HELPERS_INCLUDE

class Helpers
{
public:
	static double fRand(double fMin, double fMax);
	static void print_debug(const char * s);

	enum SongElement { NIL, INTRO, VERSE, CHORUS, BRIDGE, OUTRO };
	struct SongStructure
	{
		SongElement element;
		unsigned int elemNumber;
	public:
		SongStructure() {};
		SongStructure(SongElement elem, unsigned int num);
		void set_structure(SongElement elem, unsigned int num);
		SongElement get_element();
		unsigned int get_elemNumber();
	};
};

#endif