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