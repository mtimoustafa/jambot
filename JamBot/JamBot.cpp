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
#include "../include/gtk/gtk.h"
#include <iostream>
#include <fstream>
#include <string>

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
const gchar *textInput;
GtkWidget *window;
GtkWidget *textEntry;

static void changeProgressBar(GtkWidget *widget, gpointer data)
{
	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(data), 0.8);
}

static void testFunction(GtkWidget *widget) {
	//g_signal_new("pitch-data", G_TYPE_OBJECT, G_SIGNAL_RUN_FIRST, 0, NULL, NULL, NULL, G_TYPE_NONE, 0);
	g_signal_emit_by_name(window, "button_press_event");
}

static void openDialog(GtkWidget *button, gpointer window) {
	GtkWidget *dialog, *label;
	dialog = gtk_dialog_new_with_buttons("dialog", GTK_WINDOW(window), GTK_DIALOG_MODAL, GTK_STOCK_OPEN, GTK_RESPONSE_OK,
		GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL);
	label = gtk_label_new("You clicked the button");
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), label, 0, 0, 0);
	gtk_widget_show_all(dialog);
	gint response = gtk_dialog_run(GTK_DIALOG(dialog));
	if (response == GTK_RESPONSE_OK){
		g_print("The OK is pressed");
	}
	else {
		g_print("The cancel was pressed");
	}
	gtk_widget_destroy(dialog);
}

static void dialog_result(GtkWidget *dialog, gint resp, gpointer data){
	if (resp == GTK_RESPONSE_OK) {
		g_print("OK\n");
	}
	else {
		gtk_widget_destroy(dialog);
	}
}

static void openNoneModalDialog(GtkWidget *button, gpointer window) {
	GtkWidget *dialog, *label, *image, *hbox;
	dialog = gtk_dialog_new_with_buttons("Nonmodal dialog", GTK_WINDOW(window), GTK_DIALOG_DESTROY_WITH_PARENT, GTK_STOCK_OK,
		GTK_RESPONSE_OK, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, NULL);
	label = gtk_label_new("The buttons was clicked");
	image = gtk_image_new_from_stock(GTK_STOCK_INFO, GTK_ICON_SIZE_DIALOG);

	hbox = gtk_hbox_new(0, 0);
	gtk_box_pack_start(GTK_BOX(hbox), image, 0, 0, 0);
	gtk_box_pack_start(GTK_BOX(hbox), label, 0, 0, 0);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), hbox, 0, 0, 0);
	gtk_widget_show_all(dialog);
	g_signal_connect(dialog, "response", G_CALLBACK(dialog_result), NULL);
}

static void fileBrowse(GtkWidget *button, gpointer window) {
	GtkWidget *dialog;
	gchar *fileBuffer;
	gchar *fileName;
	gboolean readFileStatus;
	GError *error;
	dialog = gtk_file_chooser_dialog_new("Choose a file", GTK_WINDOW(window), GTK_FILE_CHOOSER_ACTION_OPEN, GTK_STOCK_OK,
		GTK_RESPONSE_OK, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, NULL);
	gtk_widget_show_all(dialog);
	gint resp = gtk_dialog_run(GTK_DIALOG(dialog));
	if (resp == GTK_RESPONSE_OK) {
		fileName = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
		ifstream file(fileName);
		string line, text;
		if (file.is_open())
		{
			while (getline(file, line))
			{
				text += line;
				text += "\n";
			}

			file.close();
			GtkTextBuffer *buffer;
			buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textEntry));
			gtk_text_buffer_set_text(buffer, text.c_str(), -1);

		}
	}
	else {
		g_print("You Pressed the cancel button");
	}
	gtk_widget_destroy(dialog);
}


static void startJamming(GtkWidget *button) {
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
	hThreadArray[WAVGEN_THREAD_ARR_ID] = CreateThread(
		NULL,
		0,
		WavGenThread,
		NULL,
		0,
		&dwThreadArray[WAVGEN_THREAD_ARR_ID]);
	if (hThreadArray[WAVGEN_THREAD_ARR_ID] == NULL)
	{
		ErrorHandler(TEXT("CreateThread"));
		CloseAllThreads();
		ExitProcess(3);
	}
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
}

static void startEmerson(GtkWidget *button) {

}

static gboolean delete_event(GtkWidget *widget, GdkEvent *event, gpointer data)
{
	/* If you return FALSE in the "delete-event" signal handler,
	* GTK will emit the "destroy" signal. Returning TRUE means
	* you don't want the window to be destroyed.
	* This is useful for popping up 'are you sure you want to quit?'
	* type dialogs. */

	g_print("delete event occurred\n");

	/* Change TRUE to FALSE and the main window will be destroyed with
	* a "delete-event". */

	return TRUE;
}

/* Another callback */
static void destroy(GtkWidget *widget,
	gpointer   data)
{
	gtk_main_quit();
}

int gtkStart(int argc, char* argv[])
{
	GtkWidget *window;
	GtkWidget *windowBox, *songProgressBox, *songControlBox, *songLyricsBox, *songInputBox, *jambox;
	GtkWidget *button, *playButton, *progressBar, *progressBarTest, *showModalDialog, *showNonmodalDialog, *fileSelectDialog, *emersonButton;
	GtkWidget *startJambot;
	GtkWidget *fileBrowser;

	gtk_init(&argc, &argv);

	/*=========================== Window ===========================*/
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	g_signal_connect(window, "delete-event", G_CALLBACK(delete_event), NULL);
	g_signal_connect(window, "destroy", G_CALLBACK(destroy), NULL);

	gtk_container_set_border_width(GTK_CONTAINER(window), 200);
	gtk_window_set_title(GTK_WINDOW(window), "JamBot");

	g_signal_connect(window, "button_press_event", G_CALLBACK(changeProgressBar), NULL);

	/*=========================== Widget boxes ===========================*/
	windowBox = gtk_hbox_new(false, 0);
	gtk_widget_set_size_request(windowBox, 700, 30);

	songLyricsBox = gtk_vbox_new(false, 0);
	gtk_widget_set_size_request(songLyricsBox, 300, 50);
	gtk_box_pack_start(GTK_BOX(windowBox), songLyricsBox, false, false, 5);

	songInputBox = gtk_vbox_new(false, 0);
	gtk_box_pack_start(GTK_BOX(windowBox), songInputBox, false, false, 5);

	jambox = gtk_hbox_new(false, 0);
	gtk_widget_set_size_request(jambox, 150, 30);
	gtk_box_pack_start(GTK_BOX(windowBox), jambox, false, false, 5);

	songControlBox = gtk_hbox_new(true, 0);
	gtk_widget_set_size_request(songControlBox, 150, 30);
	gtk_box_pack_start(GTK_BOX(songInputBox), songControlBox, false, false, 5);

	songProgressBox = gtk_hbox_new(false, 0);
	gtk_widget_set_size_request(songProgressBox, 100, 30);
	gtk_box_pack_start(GTK_BOX(songInputBox), songProgressBox, false, false, 5);

	/*=========================== Progress bar ===========================*/
	progressBar = gtk_progress_bar_new();
	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(progressBar), 0.2);
	gtk_progress_bar_set_text(GTK_PROGRESS_BAR(progressBar), "progress bar");
	gtk_box_pack_start(GTK_BOX(songProgressBox), progressBar, false, false, 5);

	/*=========================== Text entry ===========================*/
	textEntry = gtk_text_view_new();
	GtkTextBuffer *buffer;
	buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textEntry));
	gtk_text_buffer_set_text(buffer, "Hello this is some text\n hello", -1);
	gtk_box_pack_start(GTK_BOX(songLyricsBox), textEntry, false, false, 5);

	/*=========================== Button ===========================*/
	GtkWidget *playArrow;
	playArrow = gtk_arrow_new(GTK_ARROW_RIGHT, GTK_SHADOW_NONE);

	playButton = gtk_button_new();
	gtk_container_add(GTK_CONTAINER(playButton), playArrow);
	gtk_box_pack_start(GTK_BOX(songControlBox), playButton, false, false, 5);
	gtk_widget_show(playArrow);
	gtk_widget_show(playButton);

	playButton = gtk_button_new_with_label("Pause");
	gtk_box_pack_start(GTK_BOX(songControlBox), playButton, false, false, 5);
	gtk_widget_show(playButton);

	playButton = gtk_button_new_with_label("Stop");
	gtk_box_pack_start(GTK_BOX(songControlBox), playButton, false, false, 5);
	gtk_widget_show(playButton);

	fileSelectDialog = gtk_button_new_with_label("File Select");
	gtk_box_pack_start(GTK_BOX(songLyricsBox), fileSelectDialog, false, false, 5);
	g_signal_connect(GTK_OBJECT(fileSelectDialog), "clicked", G_CALLBACK(fileBrowse), window);

	progressBarTest = gtk_button_new_with_label("Test");
	gtk_box_pack_start(GTK_BOX(songProgressBox), progressBarTest, false, false, 5);
	g_signal_connect(GTK_OBJECT(progressBarTest), "clicked", G_CALLBACK(testFunction), (gpointer)progressBar);

	showModalDialog = gtk_button_new_with_label("Show modal dialog");
	gtk_box_pack_start(GTK_BOX(songProgressBox), showModalDialog, false, false, 5);
	g_signal_connect(GTK_OBJECT(showModalDialog), "clicked", G_CALLBACK(openDialog), window);

	showNonmodalDialog = gtk_button_new_with_label("Show nonmodal dialog");
	gtk_box_pack_start(GTK_BOX(songProgressBox), showNonmodalDialog, false, false, 5);
	g_signal_connect(GTK_OBJECT(showNonmodalDialog), "clicked", G_CALLBACK(openNoneModalDialog), window);

	startJambot = gtk_button_new_with_label("Start system as normal");
	gtk_box_pack_start(GTK_BOX(jambox), startJambot, false, false, 5);
	g_signal_connect(GTK_OBJECT(startJambot), "clicked", G_CALLBACK(startJamming), window);

	emersonButton = gtk_button_new_with_label("Emerson Button");
	gtk_box_pack_start(GTK_BOX(jambox), emersonButton, false, false, 5);
	g_signal_connect(GTK_OBJECT(emersonButton), "clicked", G_CALLBACK(emersonButton), window);

	/*=========================== File Browser ===========================*/
	/*fileBrowser = gtk_file_chooser_dialog_new("File selection", GTK_WINDOW(window), GTK_FILE_CHOOSER_ACTION_OPEN, GTK_STOCK_OK,
	GTK_RESPONSE_OK, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, NULL);
	gtk_file_selection_set_filename(GTK_FILE_SELECTION(fileBrowser), "penguin.png");
	gtk_box_pack_start(GTK_BOX(windowBox), fileBrowser, false, false, 5);*/

	/*=========================== the rest ===========================*/
	gtk_container_add(GTK_CONTAINER(window), windowBox);

	gtk_widget_show_all(window);
	gtk_main();

	return 0;
}

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
	gtkStart(0, NULL);
	// Initialize global strings
	/*LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
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

	return (int) msg.wParam;*/
	return 0;
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

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_JAMBOT));
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCE(IDC_JAMBOT);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

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
		wmId = LOWORD(wParam);
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
			hThreadArray[WAVGEN_THREAD_ARR_ID] = CreateThread(
				NULL,
				0,
				WavGenThread,
				NULL,
				0,
				&dwThreadArray[WAVGEN_THREAD_ARR_ID]);
			if (hThreadArray[WAVGEN_THREAD_ARR_ID] == NULL)
			{
				ErrorHandler(TEXT("CreateThread"));
				CloseAllThreads();
				ExitProcess(3);
			}
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