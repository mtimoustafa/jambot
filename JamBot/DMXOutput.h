#include "stdafx.h"
#include <queue>
#include <stdlib.h>
#include <stdio.h>
#include <windows.h>
#include <process.h>
#include <assert.h>
#include <tchar.h>
#include <time.h>
#include <conio.h>
#include "Ftd2xx.h"
#include "Constructs.h"

#ifdef _MSC_VER
#include "ms_stdint.h"
#else
#include <stdint.h>
#endif

class DMXOutput
{
	FT_HANDLE handle;
	bool done;
	bool connected;
	int bytesWritten;
	FT_STATUS status;
	int packet_size;

	unsigned char BITS_8;
	unsigned char STOP_BITS_2;
	unsigned char Parity_None;
	uint16_t FLOW_NONE;
	unsigned char PURGE_RX;
	unsigned char PURGE_TX;


public:
	DMXOutput();
	void start();
	void init();

	static bool updateLightsOutputQueue(LightsInfo output);
	void stop();
	void close();

private: 
	int write(FT_HANDLE handle, unsigned char* data, int length);
	int start_listening(void);
};