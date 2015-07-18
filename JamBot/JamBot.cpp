// JamBot.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "JamBot.h"
#include "OptiAlgo.h"
#include "InputChannelReader.h"
#include "DMXOutput.h"
#include "WavManipulation.h"
#include "strsafe.h"

#define MAX_LOADSTRING 100

// Objects definition - name + id
#define IDC_OPTIALGOTEST_BUTTON 101
#define IDC_AUDIOINPUTTESTSTART_BUTTON 102
#define IDC_AUDIOINPUTTESTSTOP_BUTTON 103
#define IDC_WAVGENTEST_BUTTON 104
#define IDC_LIGHTTEST_BUTTON 199

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
OptiAlgo optiAlgo;
DMXOutput lightsTest;
InputChannelReader inputChannelReader = InputChannelReader();
WavManipulation wavmanipulation = WavManipulation();

// Functions to run components in threads (because WINAPI lol)
DWORD WINAPI AudioInputThreadStartFn(LPVOID lpParam) { inputChannelReader.start(); return 0; }
DWORD WINAPI AudioInputThreadStopFn(LPVOID lpParam) { inputChannelReader.stop(); return 0; }
DWORD WINAPI WavGenThreadStartFn(LPVOID lpParam) { wavmanipulation.startSnip(); return 0; }
DWORD WINAPI OptiAlgoThreadStartFn(LPVOID lpParam) { OptiAlgo optiAlgo = OptiAlgo(); optiAlgo.test_algo(); return 0; }
DWORD WINAPI AudioOutputThreadStartFn(LPVOID lpParam) { lightsTest = DMXOutput(); lightsTest.start(); return 0; }


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
	HWND hWndOptiAlgoButton, hWndAudioInputButton, hWndWavGenButton, hWndDMXLightsButton;

	switch (message)
	{
	case WM_CREATE:
		// Create buttons
		hWndOptiAlgoButton = CreateWindowEx(NULL,
			_T("BUTTON"),
			_T("Test Optimization Algorithm"),
			WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
			10,
			10,
			250,
			24,
			hWnd,
			(HMENU)IDC_OPTIALGOTEST_BUTTON,
			GetModuleHandle(NULL),
			NULL);
		hWndAudioInputButton = CreateWindowEx(NULL,
			_T("BUTTON"),
			_T("Start Audio Input"),
			WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
			10,
			40,
			120,
			24,
			hWnd,
			(HMENU)IDC_AUDIOINPUTTESTSTART_BUTTON,
			GetModuleHandle(NULL),
			NULL);
		hWndAudioInputButton = CreateWindowEx(NULL,
			_T("BUTTON"),
			_T("Stop Audio Input "),
			WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
			140,
			40,
			120,
			24,
			hWnd,
			(HMENU)IDC_AUDIOINPUTTESTSTOP_BUTTON,
			GetModuleHandle(NULL),
			NULL);
		hWndWavGenButton = CreateWindowEx(NULL,
			_T("BUTTON"),
			_T("Test Wave Generation"),
			WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
			10,
			70,
			250,
			24,
			hWnd,
			(HMENU)IDC_WAVGENTEST_BUTTON,
			GetModuleHandle(NULL),
			NULL);
		hWndDMXLightsButton = CreateWindowEx(NULL,
			_T("BUTTON"),
			_T("Test DMX Lights"),
			WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
			10,
			100,
			250,
			24,
			hWnd,
			(HMENU)IDC_LIGHTTEST_BUTTON,
			GetModuleHandle(NULL),
			NULL);

		break;
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case IDC_OPTIALGOTEST_BUTTON:
			// Run optimization algorithm test
			hThreadArray[OPTIALGO_THREAD_ARR_ID] = CreateThread(
				NULL,
				0,
				OptiAlgoThreadStartFn,
				NULL,
				0,
				&dwThreadArray[OPTIALGO_THREAD_ARR_ID]);
			if (hThreadArray[OPTIALGO_THREAD_ARR_ID] == NULL)
			{
				ErrorHandler(TEXT("CreateThread"));
				CloseAllThreads();
				ExitProcess(3);
			}
			break;
		case IDC_AUDIOINPUTTESTSTART_BUTTON:
			// Do audio input start test
			hThreadArray[AUDIOINPUT_THREAD_ARR_ID] = CreateThread(
				NULL,
				0,
				AudioInputThreadStartFn,
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
		case IDC_AUDIOINPUTTESTSTOP_BUTTON:
			// Do audio input stop test
			hThreadArray[AUDIOINPUT_THREAD_ARR_ID] = CreateThread(
				NULL,
				0,
				AudioInputThreadStopFn,
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
		case IDC_WAVGENTEST_BUTTON:
			hThreadArray[WAVGEN_THREAD_ARR_ID] = CreateThread(
				NULL,
				0,
				WavGenThreadStartFn,
				NULL,
				0,
				&dwThreadArray[WAVGEN_THREAD_ARR_ID]);
			if (hThreadArray[WAVGEN_THREAD_ARR_ID] == NULL)
			{
				ErrorHandler(TEXT("CreateThread"));
				CloseAllThreads();
				ExitProcess(3);
			}
			break;
		case IDC_LIGHTTEST_BUTTON:
			// Test DMX lights
			hThreadArray[AUDIOOUTPUT_THREAD_ARR_ID] = CreateThread(
				NULL,
				0,
				AudioOutputThreadStartFn,
				NULL,
				0,
				&dwThreadArray[AUDIOOUTPUT_THREAD_ARR_ID]);
			if (hThreadArray[AUDIOOUTPUT_THREAD_ARR_ID] == NULL)
			{
				ErrorHandler(TEXT("CreateThread"));
				CloseAllThreads();
				ExitProcess(3);
			}
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
	for (int i = 0; i<MAX_THREADS; i++)
	{
		CloseHandle(hThreadArray[i]);
	}
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