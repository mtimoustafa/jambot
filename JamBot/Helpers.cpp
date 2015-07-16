#include "stdafx.h"
#include "stdlib.h"
#include <cmath>
#include <sstream>

#include "Helpers.h"
using namespace std;

// Generate a random floating point number between fMin and fMax
double Helpers::fRand(double fMin, double fMax)
{
	double f = (double)rand() / RAND_MAX;
	return fMin + f * (fMax - fMin);

}

// Prints to the Visual Studio output window
void Helpers::print_debug(const char * s)
{
	wstringstream os;
	os << s;
	OutputDebugString(os.str().c_str());
}

#pragma region SongStructure

Helpers::SongStructure::SongStructure(SongElement elem, unsigned int num)
{
	set_structure(elem, num);
}

void Helpers::SongStructure::set_structure(SongElement elem, unsigned int num)
{
	this->element = elem;
	this->elemNumber = num;
}

Helpers::SongElement Helpers::SongStructure::get_element()
{
	return this->element;
}
unsigned int Helpers::SongStructure::get_elemNumber()
{
	return this->elemNumber;
}

#pragma endregion