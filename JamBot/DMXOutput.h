#include "stdafx.h"
#include <deque>
#include <stdlib.h>
#include <stdio.h>
#include <windows.h>
#include <process.h>
#include <assert.h>
#include <tchar.h>
#include <time.h>
#include <conio.h>
#include "Ftd2xx.h"

#ifdef _MSC_VER
#include "ms_stdint.h"
#else
#include <stdint.h>
#endif
#define byte unsigned char;
class DMXOutput
{
public:
	DMXOutput();
	void start();


private:
	int write(FT_HANDLE handle, unsigned char* data, int length);
	int main(void);
};