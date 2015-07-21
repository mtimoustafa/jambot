#include "stdafx.h"
#include "Helpers.h"
#include "DMXOutput.h"
#include <string>
#include <deque>
#include <exception>

using namespace std;

static std::queue<LightsInfo> lightsOutput;

DMXOutput::DMXOutput()
{

	handle = NULL;
	done = false;
	connected = false;
	bytesWritten = 0;
	packet_size = 513;

	BITS_8 = 8;
	STOP_BITS_2 = 2;
	Parity_None = 0;
	FLOW_NONE = 0;
	PURGE_RX = 1;
	PURGE_TX = 2;

};

void DMXOutput::start()
{
	init();
	start_listening();
}
//initilizing the hander and the status
void DMXOutput::init()
{
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
}

int DMXOutput::write(FT_HANDLE handle, unsigned char* data, int length)
{
	FT_STATUS status;
	DWORD bytesWritten;

	status = FT_Write(handle, data, length, &bytesWritten);
	if (status == FT_OK)
	{
		lightsOutput.pop();
	}
	return status;
}

bool DMXOutput::updateLightsOutputQueue(LightsInfo output)
{
	bool result = false;
	char * err_str;
	try
	{
		lightsOutput.push(output);
		result = true;
	}
	catch (exception e)
	{
		err_str = "";
		strcat(err_str, "ERROR: DMXOutput: ");
		strcat(err_str, e.what());
		strcat(err_str, "\n");
		Helpers::print_debug(err_str);
	}
	return result;
}

int DMXOutput::start_listening()
{
	//writeData
	if (status == FT_OK)
	{
		status = FT_SetBreakOn(handle);
		status = FT_SetBreakOff(handle);
		while (true)
		{
			while (lightsOutput.size() == 0)
			{
				Sleep(200);
			}
			bytesWritten = write(handle, buffer, packet_size);
		}
	}
	return 0;
}

