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
#include "gtk/gtk.h"
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
DWORD WINAPI WavGenThread(LPVOID lpParam) { wavmanipulation = WavManipulation(); Helpers::print_debug("START wav manip.\n"); wavmanipulation.start(); return 0; }
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
GtkWidget *textEntry, *sectionNameBox, *sectionTimeBox;

static void changeProgressBar(GtkWidget *widget, gpointer data)
{
	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(data), 0.8);
}

static void testFunction(GtkWidget *widget) {
	//g_signal_new("pitch-data", G_TYPE_OBJECT, G_SIGNAL_RUN_FIRST, 0, NULL, NULL, NULL, G_TYPE_NONE, 0);
	g_signal_emit_by_name(window, "button_press_event");
}

static void addNewSection(GtkWidget *widget)
{
	GtkWidget * tempEntry;
	tempEntry = gtk_entry_new();
	gtk_box_pack_start(GTK_BOX(sectionNameBox), tempEntry, false, false, 5);
	tempEntry = gtk_entry_new();
	gtk_box_pack_start(GTK_BOX(sectionTimeBox), tempEntry, false, false, 5); 
}

static void displayLyrics(GtkWidget *widget, gpointer window)
{
	GtkWidget *dialog, *label, *lyricsEntry;
	dialog = gtk_dialog_new_with_buttons("Lyrics Display", GTK_WINDOW(window), GTK_DIALOG_MODAL, NULL, NULL,
		NULL, NULL);
	

	GtkTextBuffer *buffer;
	lyricsEntry = gtk_text_view_new();
	buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(lyricsEntry));
	gtk_text_buffer_set_text(buffer, "Somebody once told me the world is gonna roll me\nI ain't the sharpest tool in the shed\nShe was looking kind of dumb with her finger and her thumb\nIn the shape of an 'L' on her forehead", -1);
	gtk_text_view_set_border_window_size(GTK_TEXT_VIEW(lyricsEntry), GTK_TEXT_WINDOW_TEXT, 30);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), lyricsEntry, false, false, 5);


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

static void fileBrowse(GtkWidget *button, gpointer window) {
	GtkWidget *dialog;
	gchar *fileName;
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
	wavmanipulation.startanalysis();
}

static gboolean delete_event(GtkWidget *widget, GdkEvent *event, gpointer data)
{
	/* If you return FALSE in the "delete-event" signal handler,
	* GTK will emit the "destroy" signal. Returning TRUE means
	* you don't want the window to be destroyed.
	* This is useful for popping up 'are you sure you want to quit?'
	* type dialogs. */
	CloseAllThreads();
	
	g_print("delete event occurred\n");

	/* Change TRUE to FALSE and the main window will be destroyed with
	* a "delete-event". */

	return false;
}

/* Another callback */
static void destroy(GtkWidget *widget,
	gpointer   data)
{
	CloseAllThreads();
	gtk_main_quit();
}

int gtkStart(int argc, char* argv[])
{
	GtkWidget *window;
	GtkWidget *windowBox, *songProgressBox, *songControlBox, *songLyricsBox, *songInputBox, *jambox, *windowBox2, *windowBox3, *graphBox, *voiceProgressBox;
	GtkWidget *playButton, *progressBar, *progressBarTest, *showModalDialog, *showNonmodalDialog, *fileSelectDialog, *emersonButton;
	GtkWidget *startJambot;
	GtkWidget *label;


	gtk_init(&argc, &argv);

	/*=========================== Window ===========================*/
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	g_signal_connect(window, "delete-event", G_CALLBACK(delete_event), NULL);
	g_signal_connect(window, "destroy", G_CALLBACK(destroy), NULL);

	//gtk_container_set_border_width(GTK_CONTAINER(window), 150);
	gtk_window_set_title(GTK_WINDOW(window), "JamBot");

	g_signal_connect(window, "dontcallthis", G_CALLBACK(changeProgressBar), NULL);

	/*=========================== Widget boxes ===========================*/
	windowBox2 = gtk_vbox_new(false, 0);
	//gtk_widget_set_size_request(windowBox2, 200, 30);

	jambox = gtk_hbox_new(false, 0);
	gtk_box_pack_start(GTK_BOX(windowBox2), jambox, false, false, 5);
	
	startJambot = gtk_button_new_with_label("Start system as normal");
	//gtk_box_pack_start(GTK_BOX(jambox), startJambot, false, false, 5);
	g_signal_connect(GTK_OBJECT(startJambot), "clicked", G_CALLBACK(startJamming), window);	

	emersonButton = gtk_button_new_with_label("Emerson Button");
	//gtk_box_pack_start(GTK_BOX(jambox), emersonButton, false, false, 5);
	g_signal_connect(GTK_OBJECT(emersonButton), "clicked", G_CALLBACK(startEmerson), window);

	windowBox = gtk_hbox_new(false, 0);
	//gtk_widget_set_size_request(windowBox, 80, 30);
	gtk_box_pack_start(GTK_BOX(windowBox2), windowBox, false, false, 5);

	windowBox3 = gtk_hbox_new(false, 0);
	//gtk_widget_set_size_request(windowBox, 150, 30);
	gtk_box_pack_start(GTK_BOX(windowBox2), windowBox3, false, false, 5);

	graphBox = gtk_vbox_new(false, 0);
	gtk_box_pack_start(GTK_BOX(windowBox3), graphBox, false, false, 5);

	songLyricsBox = gtk_vbox_new(false, 0);
	//gtk_widget_set_size_request(songLyricsBox, 200, 30);
	gtk_box_pack_start(GTK_BOX(windowBox), songLyricsBox, false, false, 5);

	songInputBox = gtk_vbox_new(false, 0);
	gtk_box_pack_start(GTK_BOX(windowBox), songInputBox, false, false, 5);

	songControlBox = gtk_hbox_new(true, 0);	
	//gtk_widget_set_size_request(songControlBox, 80, 30);
	gtk_box_pack_start(GTK_BOX(songInputBox), songControlBox, false, false, 5);

	voiceProgressBox = gtk_vbox_new(false, 0);
	//gtk_widget_set_size_request(voiceProgressBox, 100, 30);

	songProgressBox = gtk_vbox_new(false, 0);
	//gtk_widget_set_size_request(songProgressBox, 100, 30);

	/*=========================== Graph Part ===========================*/
	GtkWidget *sectionBox, *sectionLabelName, *sectionLabelTime, *addSectionButton, *submitSectionButton, 
		*sectionButtonBox, *tempEntry;
	void *data[2];

	sectionButtonBox = gtk_hbox_new(false, 0);
	sectionBox = gtk_vbox_new(false, 0);
	sectionNameBox = gtk_hbox_new(false, 0);
	sectionTimeBox = gtk_hbox_new(false, 0);

	/*sectionButtonBox*/
	addSectionButton = gtk_button_new_with_label("Add Section");
	g_signal_connect(GTK_OBJECT(addSectionButton), "clicked", G_CALLBACK(addNewSection), (gpointer) sectionNameBox, (gpointer) sectionTimeBox);
	gtk_box_pack_start(GTK_BOX(sectionButtonBox), addSectionButton, false, false, 5);
	submitSectionButton = gtk_button_new_with_label("Submit");
	gtk_box_pack_start(GTK_BOX(sectionButtonBox), submitSectionButton, false, false, 5);
	fileSelectDialog = gtk_button_new_with_label("Select Audio File");
	gtk_box_pack_start(GTK_BOX(sectionButtonBox), fileSelectDialog, false, false, 5);



	/*add sectionNameBox contents*/
	sectionLabelName = gtk_label_new("Section Name: ");
	gtk_box_pack_start(GTK_BOX(sectionNameBox), sectionLabelName, false, false, 5);
	tempEntry = gtk_entry_new();
	gtk_box_pack_start(GTK_BOX(sectionNameBox), tempEntry, false, false, 5);
	tempEntry = gtk_entry_new();
	gtk_box_pack_start(GTK_BOX(sectionNameBox), tempEntry, false, false, 5);

	/*add sectionTimeBox contents*/
	sectionLabelTime = gtk_label_new("Section Time:  ");
	gtk_box_pack_start(GTK_BOX(sectionTimeBox), sectionLabelTime, false, false, 5);
	tempEntry = gtk_entry_new();
	gtk_box_pack_start(GTK_BOX(sectionTimeBox), tempEntry, false, false, 5);
	tempEntry = gtk_entry_new();
	gtk_box_pack_start(GTK_BOX(sectionTimeBox), tempEntry, false, false, 5);


	gtk_box_pack_start(GTK_BOX(sectionBox), sectionButtonBox, false, false, 5);
	gtk_box_pack_start(GTK_BOX(sectionBox), sectionNameBox, false, false, 5);
	gtk_box_pack_start(GTK_BOX(sectionBox), sectionTimeBox, false, false, 5);
	gtk_box_pack_start(GTK_BOX(graphBox), sectionBox, false, false, 5);

	

	/*=========================== Progress bar ===========================*/
	GtkWidget * tabs;
	tabs = gtk_notebook_new();

	GtkWidget *label_voice;
	label_voice = gtk_label_new("Voice");

	GtkWidget *label_song;
	label_song = gtk_label_new("Song");

	GtkWidget *hbox, *temp, *tempButton;
	tempButton = gtk_button_new_with_label("TEMP");

	gtk_notebook_append_page(GTK_NOTEBOOK(tabs), voiceProgressBox, label_voice);
	gtk_notebook_append_page(GTK_NOTEBOOK(tabs), songProgressBox, label_song);
	gtk_box_pack_start(GTK_BOX(windowBox), tabs, false, false, 5);

	hbox = gtk_hbox_new(false, 0);
	label = gtk_label_new("Intensity (dB):");
	gtk_box_pack_start(GTK_BOX(hbox), label, false, false, 5);
	temp = gtk_entry_new();
	gtk_box_pack_start(GTK_BOX(hbox), temp, false, false, 5);
	gtk_box_pack_start(GTK_BOX(voiceProgressBox), hbox, false, false, 5);

	hbox = gtk_hbox_new(false, 0);
	label = gtk_label_new("Pitch (kHz):");
	gtk_box_pack_start(GTK_BOX(hbox), label, false, false, 5);
	temp = gtk_entry_new();
	gtk_box_pack_start(GTK_BOX(hbox), temp, false, false, 5);
	gtk_box_pack_start(GTK_BOX(voiceProgressBox), hbox, false, false, 5);

	hbox = gtk_hbox_new(false, 0);
	label = gtk_label_new("Tempo (bbm):");
	gtk_box_pack_start(GTK_BOX(hbox), label, false, false, 5);
	temp = gtk_entry_new();
	gtk_box_pack_start(GTK_BOX(hbox), temp, false, false, 5);
	gtk_box_pack_start(GTK_BOX(voiceProgressBox), hbox, false, false, 5);

	hbox = gtk_hbox_new(false, 0);
	label = gtk_label_new("Intensity (dB):");
	gtk_box_pack_start(GTK_BOX(hbox), label, false, false, 5);
	temp = gtk_entry_new();
	gtk_box_pack_start(GTK_BOX(hbox), temp, false, false, 5);
	gtk_box_pack_start(GTK_BOX(songProgressBox), hbox, false, false, 5);

	hbox = gtk_hbox_new(false, 0);
	label = gtk_label_new("Pitch (kHz):");
	gtk_box_pack_start(GTK_BOX(hbox), label, false, false, 5);
	temp = gtk_entry_new();
	gtk_box_pack_start(GTK_BOX(hbox), temp, false, false, 5);
	gtk_box_pack_start(GTK_BOX(songProgressBox), hbox, false, false, 5);

	hbox = gtk_hbox_new(false, 0);
	label = gtk_label_new("Tempo (bbm):");
	gtk_box_pack_start(GTK_BOX(hbox), label, false, false, 5);
	temp = gtk_entry_new();
	gtk_box_pack_start(GTK_BOX(hbox), temp, false, false, 5);
	gtk_box_pack_start(GTK_BOX(songProgressBox), hbox, false, false, 5);


	/*progressBar = gtk_progress_bar_new();
	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(progressBar), 0.2);
	gtk_progress_bar_set_text(GTK_PROGRESS_BAR(progressBar), "Intensity");
	gtk_box_pack_start(GTK_BOX(songProgressBox), progressBar, false, false, 5);

	progressBar = gtk_progress_bar_new();
	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(progressBar), 0.8);
	gtk_progress_bar_set_text(GTK_PROGRESS_BAR(progressBar), "Pitch");
	gtk_box_pack_start(GTK_BOX(songProgressBox), progressBar, false, false, 5);

	progressBar = gtk_progress_bar_new();
	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(progressBar), 0.5);
	gtk_progress_bar_set_text(GTK_PROGRESS_BAR(progressBar), "Tempo");
	gtk_box_pack_start(GTK_BOX(songProgressBox), progressBar, false, false, 5);*/

	playButton = gtk_button_new_with_label("Play");
	gtk_box_pack_start(GTK_BOX(songControlBox), playButton, false, false, 5);
	gtk_widget_show(playButton);

	playButton = gtk_button_new_with_label("Stop");
	gtk_box_pack_start(GTK_BOX(songControlBox), playButton, false, false, 5);
	g_signal_connect(GTK_OBJECT(playButton), "clicked", G_CALLBACK(CloseAllThreads), NULL);
	gtk_widget_show(playButton);

	playButton = gtk_button_new_with_label("Record");
	gtk_box_pack_start(GTK_BOX(songControlBox), playButton, false, false, 5);
	gtk_widget_show(playButton);

	GtkWidget *temphBox;
	temphBox = gtk_hbox_new(false, 0);

	fileSelectDialog = gtk_button_new_with_label("Select Lyrics");
	gtk_box_pack_start(GTK_BOX(temphBox), fileSelectDialog, false, false, 5);
	g_signal_connect(GTK_OBJECT(fileSelectDialog), "clicked", G_CALLBACK(fileBrowse), window);

	GtkWidget *outputLyrics;
	outputLyrics = gtk_button_new_with_label("Display Lyrics");
	g_signal_connect(GTK_OBJECT(outputLyrics), "clicked", G_CALLBACK(displayLyrics), window);
	gtk_box_pack_start(GTK_BOX(temphBox), outputLyrics, false, false, 5);

	gtk_box_pack_start(GTK_BOX(songLyricsBox), temphBox, false, false, 5);

	/*progressBarTest = gtk_button_new_with_label("Test");
	gtk_box_pack_start(GTK_BOX(songProgressBox), progressBarTest, false, false, 5);
	g_signal_connect(GTK_OBJECT(progressBarTest), "clicked", G_CALLBACK(testFunction), (gpointer)progressBar);*/

	/*=========================== Text entry ===========================*/
	textEntry = gtk_text_view_new();
	GtkTextBuffer *buffer;
	buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textEntry));
	gtk_text_buffer_set_text(buffer, "Hello this is some text\n hello", -1);
	gtk_text_view_set_border_window_size(GTK_TEXT_VIEW(textEntry), GTK_TEXT_WINDOW_TEXT, 30);
	gtk_box_pack_start(GTK_BOX(songLyricsBox), textEntry, false, false, 5);
	
	GtkWidget *scroll;
	GtkWidget *table;

	table = gtk_table_new(2, 2, FALSE);

	scroll = gtk_vscrollbar_new(gtk_text_view_get_vadjustment(GTK_TEXT_VIEW(textEntry)));

	gtk_table_attach(GTK_TABLE(table), textEntry, 0, 1, 0, 1, GTK_EXPAND, GTK_EXPAND, 0, 0);
	gtk_table_attach(GTK_TABLE(table), scroll, 1, 2, 0, 1, GTK_FILL, GTK_EXPAND, 0, 0);

	gtk_box_pack_start(GTK_BOX(songLyricsBox), scroll, false, false, 5);


	/*=========================== File Browser ===========================*/
	/*fileBrowser = gtk_file_chooser_dialog_new("File selection", GTK_WINDOW(window), GTK_FILE_CHOOSER_ACTION_OPEN, GTK_STOCK_OK,
	GTK_RESPONSE_OK, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, NULL);
	gtk_file_selection_set_filename(GTK_FILE_SELECTION(fileBrowser), "penguin.png");
	gtk_box_pack_start(GTK_BOX(windowBox), fileBrowser, false, false, 5);*/

	/*=========================== the rest ===========================*/
	gtk_container_add(GTK_CONTAINER(window), windowBox2);

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

	gtkStart(0, NULL);
	int i = 0;
	return 0;
}



void CloseThread(int id)
{
	CloseHandle(hThreadArray[id]);
}
void CloseAllThreads()
{
	DWORD result;

	if (hThreadArray[AUDIOINPUT_THREAD_ARR_ID] != NULL){
		Helpers::print_debug("Stopping audio input...\n");
		inputChannelReader.stop();
		result = WaitForSingleObject(hThreadArray[AUDIOINPUT_THREAD_ARR_ID], 10000);
		if (result == WAIT_OBJECT_0) { Helpers::print_debug("STOP audio input.\n"); }
		else if (result == WAIT_FAILED) { ErrorHandler(TEXT("WaitForSingleObject")); }
		else { Helpers::print_debug("FAILED stopping audio input.\n"); }
	}

	if (hThreadArray[OPTIALGO_THREAD_ARR_ID] != NULL){
		Helpers::print_debug("Stopping optimization algorithm...\n");
		optiAlgo.stop();
		result = WaitForSingleObject(hThreadArray[OPTIALGO_THREAD_ARR_ID], 3000);
		if (result == WAIT_OBJECT_0) { Helpers::print_debug("STOP opti algo.\n"); }
		else if (result == WAIT_FAILED) { ErrorHandler(TEXT("WaitForSingleObject")); }
		else { Helpers::print_debug("FAILED stopping optimization algorithm.\n"); }
	}

	/*Helpers::print_debug("Stopping wav manip...\n");
	wavmanipulation.stop();
	result = WaitForSingleObject(hThreadArray[WAVGEN_THREAD_ARR_ID], 500);
	if (result == WAIT_OBJECT_0) { Helpers::print_debug("STOP wav manip.\n"); }
	else if (result == WAIT_FAILED) { ErrorHandler(TEXT("WaitForSingleObject")); }
	else { Helpers::print_debug("FAILED stopping wav manip.\n"); }*/

	if (hThreadArray[AUDIOOUTPUT_THREAD_ARR_ID] != NULL) {
		Helpers::print_debug("Stopping audio output...\n");
		lightsTest.stop();
		result = WaitForSingleObject(hThreadArray[AUDIOOUTPUT_THREAD_ARR_ID], 3000);
		if (result == WAIT_OBJECT_0) { Helpers::print_debug("STOP audio output.\n"); }
		else if (result == WAIT_FAILED) { ErrorHandler(TEXT("WaitForSingleObject")); }
		else { Helpers::print_debug("FAILED stopping audio output.\n"); }
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