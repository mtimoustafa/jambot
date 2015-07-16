#include "stdafx.h"
#include "Helpers.h"
#include "DMXOutput.h"
#include <string>
#include <deque>

DMXOutput::DMXOutput()
{
	
};

void DMXOutput::start()
{
	main();
}

int DMXOutput::write(FT_HANDLE handle, unsigned char* data, int length)
{
	FT_STATUS status;
	DWORD bytesWritten;
	unsigned char data2[513];
	data2[1] = 255;
	data2[2] = 255;
	data2[3] = 255;
	data2[4] = 255;
	data2[5] = 255;
	data2[6] = 255;
	data2[7] = 255;
	data2[8] = 255;
	status = FT_Write(handle, data2, length, &bytesWritten);
	return status;
}

int DMXOutput::main()
{
	static unsigned char buffer[513];
	FT_HANDLE handle = NULL;
	static bool done = false;
	static bool connected = false;
	static int bytesWritten = 0;
	static FT_STATUS status;

	const unsigned char BITS_8 = 8;
	const unsigned char STOP_BITS_2 = 2;
	const unsigned char Parity_None = 0;
	const uint16_t FLOW_NONE = 0;
	const unsigned char PURGE_RX = 1;
	const unsigned char PURGE_TX = 2;

	//start
	handle = 0;
	status = FT_Open(0, &handle);

	//initOpenDMX
	status = FT_ResetDevice(handle);
	status = FT_SetDivisor(handle, (char)12);  // set baud rate
	status = FT_SetDataCharacteristics(handle, BITS_8, STOP_BITS_2, Parity_None);
	status = FT_SetFlowControl(handle, (char)FLOW_NONE, 0, 0);
	status = FT_ClrRts(handle);
	status = FT_Purge(handle, PURGE_TX);
	status = FT_Purge(handle, PURGE_RX);

	//writeData
	if (status == FT_OK)
	{
		status = FT_SetBreakOn(handle);
		status = FT_SetBreakOff(handle);
		bytesWritten = write(handle, buffer, 513);
	}
	return 0;
}

