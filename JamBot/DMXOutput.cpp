#include "stdafx.h"
#include "Helpers.h"
#include "DMXOutput.h"
#include <string>
#include <deque>
#include <exception>

using namespace std;

static std::queue<LightsInfo> lightsOutput;
unsigned char turnOffLightsPacket[513];


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

void DMXOutput::stop()
{
	done = true;
}	

void DMXOutput::close()
{
	status = FT_Purge(handle, PURGE_TX);
	status = FT_Purge(handle, PURGE_RX);
	status = FT_Close(&handle);
	if (status != FT_OK)
	{
		Helpers::print_debug("DMX device not closed properly\n");
	}
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
	if (status != FT_OK)
	{
		Helpers::print_debug("ERROR: DMXOutput: write unsuccessful (did not receive FT_OK).\n");
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
	unsigned char* info_packet;
	//writeData
	if (status == FT_OK)
	{
		status = FT_SetBreakOn(handle);
		status = FT_SetBreakOff(handle);
		while (!done)
		{ 
			while (lightsOutput.size() == 0) { }

			info_packet = lightsOutput.front().convert_to_output();
			lightsOutput.pop();
			bytesWritten = write(handle, info_packet, packet_size);
		}
		write(handle, turnOffLightsPacket, packet_size);
	}
	return 0;
}

