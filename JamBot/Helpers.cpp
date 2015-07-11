#include "stdafx.h"
#include "stdlib.h"
#include <cmath>

#include "Helpers.h"
using namespace std;

double Helpers::fRand(double fMin, double fMax)
{
	double f = (double)rand() / RAND_MAX;
	return fMin + f * (fMax - fMin);

}