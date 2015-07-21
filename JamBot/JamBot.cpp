// JamBot.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "JamBot.h"
#include "OptiAlgo.h"
#include "InputChannelReader.h"
#include "DMXOutput.h"
#include "WavManipulation.h"
#include "Helpers.h"
#include "strsafe.h"

#define MAX_LOADSTRING 100

// Objects definition - name + id
#define IDC_STARTSYS_BUTTON 101
#define IDC_STOPSYS_BUTTON  102

// Thread-related global constants
#define MAX_THREADS 4
#define AUDIOINPUT_THREAD_ARR_ID 0
#define WAVGEN_THREAD_ARR_ID 1
#define OPTIALGO_THREAD_ARR_ID 2
#define AUDIOOUTPUT_THREAD_ARR_ID 3

// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name
HANDLE hThreadArray[MAX_THREADS];				// Array of threads
DWORD dwThreadArray[MAX_THREADS];				// Array of returned thread IDs
InputChannelReader inputChannelReader = InputChannelReader();
WavManipulation wavmanipulation = WavManipulation();
OptiAlgo optiAlgo = OptiAlgo();
DMXOutput lightsTest = DMXOutput();

// Functions to run components in threads
DWORD WINAPI AudioInputThread(LPVOID lpParam) { inputChannelReader = InputChannelReader(); Helpers::print_debug("START audio input.\n"); inputChannelReader.start(); return 0; }
DWORD WINAPI WavGenThread(LPVOID lpParam) { wavmanipulation = WavManipulation(); Helpers::print_debug("START wav manip.\n"); wavmanipulation.startSnip(); return 0; }
DWORD WINAPI OptiAlgoThread(LPVOID lpParam) { optiAlgo = OptiAlgo(); Helpers::print_debug("START opti algo.\n"); optiAlgo.start(); return 0; }
DWORD WINAPI AudioOutputThread(LPVOID lpParam) { lightsTest = DMXOutput(); Helpers::print_debug("START audio output.\n"); lightsTest.start(); return 0; }


// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);
void CloseThread(int id);
void CloseAllThreads();
void ErrorHandler(LPTSTR lpszFunction);

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPTSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

 	// TODO: Place code here.
	MSG msg;
	HACCEL hAccelTable;

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_JAMBOT, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_JAMBOT));

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_JAMBOT));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_JAMBOT);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   hInst = hInstance; // Store instance handle in our global variable

   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;
	HWND hWndStartSysButton, hWndStopSysButton;

	switch (message)
	{
	case WM_CREATE:
		// Create buttons
		hWndStartSysButton = CreateWindowEx(NULL,
			_T("BUTTON"),
			_T("Start system"),
			WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
			10,
			10,
			250,
			24,
			hWnd,
			(HMENU)IDC_STARTSYS_BUTTON,
			GetModuleHandle(NULL),
			NULL);
		hWndStopSysButton = CreateWindowEx(NULL,
			_T("BUTTON"),
			_T("Stop system"),
			WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
			10,
			40,
			250,
			24,
			hWnd,
			(HMENU)IDC_STOPSYS_BUTTON,
			GetModuleHandle(NULL),
			NULL);
		break;
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case IDC_STARTSYS_BUTTON:
			hThreadArray[AUDIOOUTPUT_THREAD_ARR_ID] = CreateThread(
				NULL,
				0,
				AudioOutputThread,
				NULL,
				0,
				&dwThreadArray[AUDIOOUTPUT_THREAD_ARR_ID]);
			if (hThreadArray[AUDIOOUTPUT_THREAD_ARR_ID] == NULL)
			{
				ErrorHandler(TEXT("CreateThread"));
				CloseAllThreads();
				ExitProcess(3);
			}
			hThreadArray[OPTIALGO_THREAD_ARR_ID] = CreateThread(
				NULL,
				0,
				OptiAlgoThread,
				NULL,
				0,
				&dwThreadArray[OPTIALGO_THREAD_ARR_ID]);
			if (hThreadArray[OPTIALGO_THREAD_ARR_ID] == NULL)
			{
				ErrorHandler(TEXT("CreateThread"));
				CloseAllThreads();
				ExitProcess(3);
			}
			//hThreadArray[WAVGEN_THREAD_ARR_ID] = CreateThread(
			//	NULL,
			//	0,
			//	WavGenThread,
			//	NULL,
			//	0,
			//	&dwThreadArray[WAVGEN_THREAD_ARR_ID]);
			//if (hThreadArray[WAVGEN_THREAD_ARR_ID] == NULL)
			//{
			//	ErrorHandler(TEXT("CreateThread"));
			//	CloseAllThreads();
			//	ExitProcess(3);
			//}
			hThreadArray[AUDIOINPUT_THREAD_ARR_ID] = CreateThread(
				NULL,
				0,
				AudioInputThread,
				NULL,
				0,
				&dwThreadArray[AUDIOINPUT_THREAD_ARR_ID]);
			if (hThreadArray[AUDIOINPUT_THREAD_ARR_ID] == NULL)
			{
				ErrorHandler(TEXT("CreateThread"));
				CloseAllThreads();
				ExitProcess(3);
			}
			break;
		case IDC_STOPSYS_BUTTON:
			CloseAllThreads();
			break;
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		// TODO: Add any drawing code here...
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		CloseAllThreads();
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

void CloseThread(int id)
{
	CloseHandle(hThreadArray[id]);
}
void CloseAllThreads()
{
	DWORD result;

	Helpers::print_debug("Stopping audio input...\n");
	inputChannelReader.stop();
	result = WaitForSingleObject(hThreadArray[AUDIOINPUT_THREAD_ARR_ID], 10000);
	if (result == WAIT_OBJECT_0) { Helpers::print_debug("STOP audio input.\n"); }
	else if (result == WAIT_FAILED) { ErrorHandler(TEXT("WaitForSingleObject")); }
	else { Helpers::print_debug("FAILED stopping audio input.\n"); }

	Helpers::print_debug("Stopping optimization algorithm...\n");
	optiAlgo.stop();
	result = WaitForSingleObject(hThreadArray[OPTIALGO_THREAD_ARR_ID], 3000);
	if (result == WAIT_OBJECT_0) { Helpers::print_debug("STOP opti algo.\n"); }
	else if (result == WAIT_FAILED) { ErrorHandler(TEXT("WaitForSingleObject")); }
	else { Helpers::print_debug("FAILED stopping optimization algorithm.\n"); }

	//Helpers::print_debug("Stopping wav manip...\n");
	////wavmanipulation.stop();
	//result = WaitForSingleObject(hThreadArray[WAVGEN_THREAD_ARR_ID], 500);
	//if (result == WAIT_OBJECT_0) { Helpers::print_debug("STOP wav manip.\n"); }
	//else if (result == WAIT_FAILED) { ErrorHandler(TEXT("WaitForSingleObject")); }
	//else { Helpers::print_debug("FAILED stopping wav manip.\n"); }

	Helpers::print_debug("Stopping audio output...\n");
	lightsTest.stop();
	result = WaitForSingleObject(hThreadArray[AUDIOOUTPUT_THREAD_ARR_ID], 3000);
	if (result == WAIT_OBJECT_0) { Helpers::print_debug("STOP audio output.\n"); }
	else if (result == WAIT_FAILED) { ErrorHandler(TEXT("WaitForSingleObject")); }
	else { Helpers::print_debug("FAILED stopping audio output.\n"); }
}

void ErrorHandler(LPTSTR lpszFunction)
{
	// Retrieve the system error message for the last-error code.

	LPVOID lpMsgBuf;
	LPVOID lpDisplayBuf;
	DWORD dw = GetLastError();

	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		dw,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf,
		0, NULL);

	// Display the error message.

	lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT,
		(lstrlen((LPCTSTR)lpMsgBuf) + lstrlen((LPCTSTR)lpszFunction) + 40) * sizeof(TCHAR));
	StringCchPrintf((LPTSTR)lpDisplayBuf,
		LocalSize(lpDisplayBuf) / sizeof(TCHAR),
		TEXT("%s failed with error %d: %s"),
		lpszFunction, dw, lpMsgBuf);
	MessageBox(NULL, (LPCTSTR)lpDisplayBuf, TEXT("Error"), MB_OK);

	// Free error-handling buffer allocations.

	LocalFree(lpMsgBuf);
	LocalFree(lpDisplayBuf);
}