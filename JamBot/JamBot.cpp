// JamBot.cpp : Defines the entry point for the application.

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
#include <deque>

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

void CloseThread(int id);
void CloseAllThreads();
void ErrorHandler(LPTSTR lpszFunction);
const gchar *textInput;
GtkWidget *window, *lyricsEntry;
GtkWidget *textEntry, *sectionNameBox, *sectionTimeBox, *songSelectBox;
GList *songList = NULL;
string waveFilePath, lyricsPath;
string lyrics, csvFileName;
GtkTextBuffer *lyricsBuffer;
deque<GtkWidget*> sectionNameDetails;
deque<GtkWidget*> sectionTimeDetails;
ofstream masterCSV;
deque<string> csvList;
GtkListStore *liststore;
bool songSelectedFlag = false;
bool graphWaveFlag = false;
GtkWidget *drawArea, *sectionDialog;
bool songSectionFlag = false;
bool alreadyJamming = false;
int songListPosition = 0;

// Functions to run components in threads
DWORD WINAPI AudioInputThread(LPVOID lpParam) { inputChannelReader = InputChannelReader(); Helpers::print_debug("START audio input.\n"); inputChannelReader.start(songSelectedFlag); return 0; }
DWORD WINAPI WavGenThread(LPVOID lpParam) { wavmanipulation = WavManipulation(); Helpers::print_debug("START wav manip.\n"); wavmanipulation.start(csvFileName); return 0; }
DWORD WINAPI OptiAlgoThread(LPVOID lpParam) { optiAlgo = OptiAlgo(); Helpers::print_debug("START opti algo.\n"); optiAlgo.start(); return 0; }
DWORD WINAPI AudioOutputThread(LPVOID lpParam) { lightsTest = DMXOutput(); Helpers::print_debug("START audio output.\n"); lightsTest.start(); return 0; }


// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);


static void changeProgressBar(GtkWidget *widget, gpointer data)
{
	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(data), 0.8);
}

void JamBot::updateLyrics(string text) {
	lyrics = text;
	lyrics += "\n";
	gtk_text_buffer_set_text(lyricsBuffer, lyrics.c_str(), -1);
}

static void testFunction(GtkWidget *widget) {
	g_signal_emit_by_name(window, "button_press_event");
}

static void dialog_result(GtkWidget *dialog, gint resp, gpointer data) {
	if (resp != GTK_RESPONSE_OK) {
		songSectionFlag = false;
		/*clears to free up membory and prevent cross referencing*/
		sectionNameDetails.clear();
		sectionTimeDetails.clear();
		gtk_widget_destroy(dialog);
	}
}

static void graphWave() {
	if (graphWaveFlag)
	{
		SoundFileRead insound((waveFilePath).c_str());
		SoundHeader header = insound;
		int sampleAmount = header.getSamples();
		int length = floor((double)insound.getSamples() / (double)insound.getSrate());
		/*getting how many samples we are going to take for this graph*/
		int sampleRate = sampleAmount / 10000;
		/*first sample*/
		int currentSample = sampleRate;
		double sampleDiff[10000];
		double sampleValue[10000];
		int i = 1;

		/*setting up where the graph will be displayed*/
		cairo_t *cr = gdk_cairo_create(drawArea->window);
		cairo_set_source_rgba(cr, 1, 0.4, 0.3, 0.8);
		cairo_set_line_width(cr, 1.3);
		/*setting the starting point of the graph*/
		cairo_move_to(cr, 0.0, 150.0);
		/*getting the value of the beginning sample*/
		insound.gotoSample(0);
		sampleValue[0] = insound.getCurrentSampleDouble(0) * 0x100;
		sampleDiff[0] = 0.00;
		for (int i = 1; i < 10000; i++)
		{
			/*go to the next sample*/
			insound.gotoSample(currentSample);
			sampleValue[i] = insound.getCurrentSampleDouble(0) * 0x100;
			sampleDiff[i] = sampleValue[i] - sampleValue[i - 1];
			cairo_rel_line_to(cr, 0.049, sampleDiff[i]);
			currentSample += sampleRate;
		}
		cairo_stroke(cr);

		cairo_move_to(cr, 0.0, 150.0);
		cairo_set_source_rgba(cr, 0, 0, 0, 1.0);
		cairo_rel_line_to(cr, 500, 0);

		cairo_move_to(cr, 0.0, 150.0);
		cairo_rel_line_to(cr, 0, 150);

		cairo_move_to(cr, 0.0, 150.0);
		cairo_rel_line_to(cr, 0, -150.0);

		/*move back to origin*/
		int tick;
		double x;
		cairo_move_to(cr, 0.0, 150.0);
		for (int i = 0; i < 16; i++) {
			x = 31.25*i;
			/*determine the "tick" length based on 5's or 10's*/
			if (i % 2)
			{
				tick = 5;
			}
			else
			{
				tick = 10;
			}

			cairo_move_to(cr, x, 150.0);
			cairo_rel_line_to(cr, 0, tick);
			cairo_rel_line_to(cr, 0, (-2*tick));
			cairo_move_to(cr, x, 150.0);
		}
		cairo_stroke(cr);

	}
}

static void addNewSection(GtkWidget *widget)
{
	GtkWidget *hbox, *tempEntry;
	
	hbox = gtk_hbox_new(false, 0);

	tempEntry = gtk_entry_new();
	sectionNameDetails.push_back(tempEntry);
	gtk_box_pack_start(GTK_BOX(hbox), tempEntry, false, false, 5);

	tempEntry = gtk_entry_new();
	sectionTimeDetails.push_back(tempEntry);
	gtk_box_pack_start(GTK_BOX(hbox), tempEntry, false, false, 5);

	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(sectionDialog)->vbox), hbox, false, false, 5);

	gtk_widget_show_all(sectionDialog);
}

static void submitSongSection() {
	GtkWidget *sectionTime, *sectionName;
	const gchar *name, *time;
	GtkCellRenderer *column;

	if (!lyricsPath.empty() && !waveFilePath.empty()) {
		vector<SongSection> section = vector<SongSection>();

		for (int i = 0; i < sectionNameDetails.size(); i++) {
			name = gtk_entry_get_text(GTK_ENTRY(sectionNameDetails[i]));
			time = gtk_entry_get_text(GTK_ENTRY(sectionTimeDetails[i]));
			section.push_back(SongSection(name, atoi((char*)time)));
		}
		int position = lyricsPath.find_last_of('/\\');
		string fileName = lyricsPath.substr(position + 1);
		fileName = fileName.substr(0, fileName.size() - 4);
		wavmanipulation.dataStore(fileName, section, waveFilePath, lyricsPath);

		masterCSV.open("CSV\\masterCSV.csv", ios_base::app);
		masterCSV << fileName + "\n";
		masterCSV.close();

		csvList.push_back(fileName);

		gtk_list_store_insert_with_values(liststore, NULL, -1, 0, "red", 1, (char*)fileName.c_str(), -1);
		gtk_dialog_response(GTK_DIALOG(sectionDialog), GTK_RESPONSE_CLOSE);

		gtk_widget_show_all(songSelectBox);
	}
}

static void displaySectionModal(GtkWidget *widget, gint resp, gpointer *data)
{
	if (!songSectionFlag)
	{
		GtkWidget *hbox, *label, *tempEntry, *addSectionButton, *submitButton;
		sectionDialog = gtk_dialog_new_with_buttons("Nonmodal dialog", GTK_WINDOW(window), GTK_DIALOG_DESTROY_WITH_PARENT, NULL,
			NULL, NULL, NULL);
		g_signal_connect(sectionDialog, "response", G_CALLBACK(dialog_result), NULL);

		hbox = gtk_hbox_new(false, 0);
		addSectionButton = gtk_button_new_with_label("Add Section");
		g_signal_connect(GTK_OBJECT(addSectionButton), "clicked", G_CALLBACK(addNewSection), NULL);
		gtk_box_pack_start(GTK_BOX(hbox), addSectionButton, false, false, 5);

		submitButton = gtk_button_new_with_label("Submit");
		g_signal_connect(GTK_OBJECT(submitButton), "clicked", G_CALLBACK(submitSongSection), (gpointer)sectionNameBox, (gpointer)sectionTimeBox);
		gtk_box_pack_start(GTK_BOX(hbox), submitButton, false, false, 5);
		gtk_box_pack_start(GTK_BOX(GTK_DIALOG(sectionDialog)->vbox), hbox, false, false, 5);

		hbox = gtk_hbox_new(false, 0);
		label = gtk_label_new("Section Name");
		gtk_box_pack_start(GTK_BOX(hbox), label, false, false, 5);
		label = gtk_label_new("Section Time");
		gtk_box_pack_start(GTK_BOX(hbox), label, false, false, 85);
		gtk_box_pack_start(GTK_BOX(GTK_DIALOG(sectionDialog)->vbox), hbox, false, false, 5);

		hbox = gtk_hbox_new(false, 0);

		tempEntry = gtk_entry_new();
		sectionNameDetails.push_back(tempEntry);
		gtk_box_pack_start(GTK_BOX(hbox), tempEntry, false, false, 5);

		tempEntry = gtk_entry_new();
		sectionTimeDetails.push_back(tempEntry);
		gtk_box_pack_start(GTK_BOX(hbox), tempEntry, false, false, 5);
		gtk_box_pack_start(GTK_BOX(GTK_DIALOG(sectionDialog)->vbox), hbox, false, false, 5);

		hbox = gtk_hbox_new(false, 0);

		tempEntry = gtk_entry_new();
		sectionNameDetails.push_back(tempEntry);
		gtk_box_pack_start(GTK_BOX(hbox), tempEntry, false, false, 5);

		tempEntry = gtk_entry_new();
		sectionTimeDetails.push_back(tempEntry);
		gtk_box_pack_start(GTK_BOX(hbox), tempEntry, false, false, 5);
		gtk_box_pack_start(GTK_BOX(GTK_DIALOG(sectionDialog)->vbox), hbox, false, false, 5);

		gtk_widget_show_all(sectionDialog);

		songSectionFlag = true;
	}
}

static void selectWaveFile(GtkWidget *widget) {
	GtkWidget *dialog;
	dialog = gtk_file_chooser_dialog_new("Choose a file", GTK_WINDOW(window), GTK_FILE_CHOOSER_ACTION_OPEN, GTK_STOCK_OK,
		GTK_RESPONSE_OK, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, NULL);
	gtk_widget_show_all(dialog);
	gint resp = gtk_dialog_run(GTK_DIALOG(dialog));
	if (resp == GTK_RESPONSE_OK) {
		waveFilePath = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
		graphWaveFlag = true;
		graphWave();
	}
	else {
		g_print("You Pressed the cancel button");
	}
	gtk_widget_destroy(dialog);
}

static void displayLyricsNonmodal(GtkWidget *widget, gpointer window)
{
	GtkWidget *dialog, *label, *image;
	dialog = gtk_dialog_new_with_buttons("Nonmodal dialog", GTK_WINDOW(window), GTK_DIALOG_DESTROY_WITH_PARENT, NULL,
		NULL, NULL, NULL);
	PangoFontDescription *font_desc;

	label = gtk_label_new("The button was clicked");

	lyricsEntry = gtk_text_view_new();
	lyricsBuffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(lyricsEntry));
	gtk_text_buffer_set_text(lyricsBuffer, lyrics.c_str(), -1);
	GtkTextIter start, end;

	gtk_text_buffer_get_start_iter(lyricsBuffer, &start);
	gtk_text_buffer_get_end_iter(lyricsBuffer, &end);

	gtk_text_buffer_create_tag(lyricsBuffer, "italic", "style", PANGO_STYLE_ITALIC, NULL);
	gtk_text_buffer_create_tag(lyricsBuffer, "bold", "weight", PANGO_WEIGHT_BOLD, NULL);

	font_desc = pango_font_description_from_string("Serif 35");
	gtk_widget_modify_font(lyricsEntry, font_desc);
	pango_font_description_free(font_desc);

	gtk_text_view_set_border_window_size(GTK_TEXT_VIEW(lyricsEntry), GTK_TEXT_WINDOW_TEXT, 30);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), lyricsEntry, false, false, 5);

	gtk_widget_show_all(dialog);
}

static void displayLyrics(GtkWidget *widget, gpointer window)
{
	GtkWidget *dialog, *label;
	PangoFontDescription *font_desc;
	dialog = gtk_dialog_new_with_buttons("Lyrics Display", GTK_WINDOW(window), GTK_DIALOG_MODAL, NULL, NULL,
		NULL, NULL);

	lyricsEntry = gtk_text_view_new();
	lyricsBuffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(lyricsEntry));

	gtk_text_buffer_set_text(lyricsBuffer, lyrics.c_str(), -1);
	GtkTextIter start, end;

	gtk_text_buffer_get_start_iter(lyricsBuffer, &start);
	gtk_text_buffer_get_end_iter(lyricsBuffer, &end);

	gtk_text_buffer_create_tag(lyricsBuffer, "italic", "style", PANGO_STYLE_ITALIC, NULL);
	gtk_text_buffer_create_tag(lyricsBuffer, "bold", "weight", PANGO_WEIGHT_BOLD, NULL);

	font_desc = pango_font_description_from_string("Serif 35");
	gtk_widget_modify_font(lyricsEntry, font_desc);
	pango_font_description_free(font_desc);

	gtk_text_buffer_apply_tag_by_name(lyricsBuffer, "italic", &start, &end);

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

static void selectLyrics(GtkWidget *button, gpointer window) {
	GtkWidget *dialog;
	gchar *fileName;
	dialog = gtk_file_chooser_dialog_new("Choose a file", GTK_WINDOW(window), GTK_FILE_CHOOSER_ACTION_OPEN, GTK_STOCK_OK,
		GTK_RESPONSE_OK, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, NULL);
	gtk_widget_show_all(dialog);
	gint resp = gtk_dialog_run(GTK_DIALOG(dialog));
	if (resp == GTK_RESPONSE_OK) {
		lyricsPath = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
	}
	else {
		g_print("You Pressed the cancel button");
	}
	gtk_widget_destroy(dialog);
}

static void startJamming(GtkWidget *button) {
	gint position = gtk_combo_box_get_active(GTK_COMBO_BOX(songSelectBox));
	csvFileName = csvList[position] + ".csv";
	if (!alreadyJamming) {

		if (position > 0 || csvFileName.compare("None.csv") != 0)
		{
			songSelectedFlag = true;
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
		}
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
		// Emerson's thread used to be created here
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
		alreadyJamming = true;
	}
}

static void startEmerson(GtkWidget *button) {
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
}

static void updateProgress()
{
	int i = 0;
	g_signal_new("pitch-data", G_TYPE_OBJECT, G_SIGNAL_RUN_FIRST, 0, NULL, NULL, NULL, G_TYPE_NONE, 0);
}

void JamBot::signalNewAnalysisValues()
{
	g_signal_emit_by_name(window, "pitch-data");
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
	GtkWidget *windowBox, *songProgressBox, *songControlBox, *songLyricsBox, *songInputBox, *jambox, *windowBox2, *windowBox3, *voiceProgressBox;
	GtkWidget *playButton, *progressBar, *progressBarTest, *showModalDialog, *showNonmodalDialog, *fileSelectDialog, *emersonButton;
	GtkWidget *startJambot, *graphBox;
	GtkWidget *label;

	gtk_init(&argc, &argv);
	lyrics = "Hi there";
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
	gtk_box_pack_start(GTK_BOX(jambox), emersonButton, false, false, 5);
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
	addSectionButton = gtk_button_new_with_label("Section");
	g_signal_connect(GTK_OBJECT(addSectionButton), "clicked", G_CALLBACK(displaySectionModal), NULL, NULL);
	gtk_box_pack_start(GTK_BOX(sectionButtonBox), addSectionButton, false, false, 5);

	fileSelectDialog = gtk_button_new_with_label("Select Audio File");
	g_signal_connect(GTK_OBJECT(fileSelectDialog), "clicked", G_CALLBACK(selectWaveFile), (gpointer)sectionNameBox, (gpointer)sectionTimeBox);
	gtk_box_pack_start(GTK_BOX(sectionButtonBox), fileSelectDialog, false, false, 5);

	gtk_box_pack_start(GTK_BOX(sectionBox), sectionButtonBox, false, false, 5);

	/*configure draw area*/
	gtk_box_pack_start(GTK_BOX(graphBox), sectionBox, false, false, 5);
	drawArea = gtk_drawing_area_new();

	gtk_widget_set_size_request(GTK_WIDGET(drawArea), 550, 300);
	gtk_window_set_resizable(GTK_WINDOW(drawArea), FALSE);

	gtk_box_pack_start(GTK_BOX(graphBox), drawArea, true, true, 0);

	g_signal_connect(drawArea, "expose-event", G_CALLBACK(graphWave), NULL);

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
	g_signal_connect(G_OBJECT(window), "pitch-data", G_CALLBACK(updateProgress), NULL);
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

	playButton = gtk_button_new_with_label("Play");
	gtk_widget_set_double_buffered(playButton, (gboolean)false);
	gtk_box_pack_start(GTK_BOX(songControlBox), playButton, false, false, 5);
	g_signal_connect(GTK_OBJECT(playButton), "clicked", G_CALLBACK(startJamming), window);
	gtk_widget_show(playButton);

	playButton = gtk_button_new_with_label("Stop");
	gtk_box_pack_start(GTK_BOX(songControlBox), playButton, false, false, 5);
	g_signal_connect(GTK_OBJECT(playButton), "clicked", G_CALLBACK(CloseAllThreads), NULL);
	gtk_widget_show(playButton);

	/*============================== COMBO  ========================================*/
	songSelectBox = gtk_combo_box_new();
	g_signal_connect(GTK_OBJECT(songSelectBox), "move-active", G_CALLBACK(testFunction), NULL);
	GtkCellRenderer *column;

	gtk_init(&argc, &argv);

	liststore = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_STRING);

	ifstream file("CSV\\masterCSV.csv");
	string line, text;
	if (!file)
	{
		ofstream ofile("CSV\\masterCSV.csv");
	}
	ifstream ifile("CSV\\masterCSV.csv");
	if (ifile.is_open())
	{
		gtk_list_store_insert_with_values(liststore, NULL, songListPosition, 0, "red", 1, "None", -1);
		csvList.push_back("None");
		songListPosition++;

		while (getline(file, line))
		{
			gtk_list_store_insert_with_values(liststore, NULL, songListPosition, 0, "red", 1, (char*)line.c_str(), -1);
			csvList.push_back(line);
			songListPosition++;
		}
		file.close();
	}
	songSelectBox = gtk_combo_box_new_with_model(GTK_TREE_MODEL(liststore));

	//g_object_unref(liststore);

	column = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(songSelectBox), column, TRUE);

	/* column does not need to be g_object_unref()ed because it
	* is GInitiallyUnowned and the floating reference has been
	* passed to combo by the gtk_cell_layout_pack_start() call. */

	gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(songSelectBox), column, "cell-background", 0, "text", 1, NULL);

	gtk_combo_box_set_active(GTK_COMBO_BOX(songSelectBox), 0);
	gtk_box_pack_start(GTK_BOX(graphBox), songSelectBox, false, false, 5);

	GtkWidget *temphBox;
	temphBox = gtk_hbox_new(false, 0);

	fileSelectDialog = gtk_button_new_with_label("Select Lyrics");
	gtk_box_pack_start(GTK_BOX(temphBox), fileSelectDialog, false, false, 5);
	g_signal_connect(GTK_OBJECT(fileSelectDialog), "clicked", G_CALLBACK(selectLyrics), window);

	GtkWidget *outputLyrics;
	outputLyrics = gtk_button_new_with_label("Display Lyrics");
	g_signal_connect(GTK_OBJECT(outputLyrics), "clicked", G_CALLBACK(displayLyricsNonmodal), window);
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
	if (alreadyJamming){
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

		if (hThreadArray[WAVGEN_THREAD_ARR_ID] != NULL){
			Helpers::print_debug("Stopping wav manip...\n");
			wavmanipulation.stop();
			result = WaitForSingleObject(hThreadArray[WAVGEN_THREAD_ARR_ID], 500);
			if (result == WAIT_OBJECT_0) { Helpers::print_debug("STOP wav manip.\n"); }
			else if (result == WAIT_FAILED) { ErrorHandler(TEXT("WaitForSingleObject")); }
			else { Helpers::print_debug("FAILED stopping wav manip.\n"); }
		}

		if (hThreadArray[AUDIOOUTPUT_THREAD_ARR_ID] != NULL) {
			Helpers::print_debug("Stopping audio output...\n");
			lightsTest.stop();
			result = WaitForSingleObject(hThreadArray[AUDIOOUTPUT_THREAD_ARR_ID], 3000);
			if (result == WAIT_OBJECT_0) { Helpers::print_debug("STOP audio output.\n"); }
			else if (result == WAIT_FAILED) { ErrorHandler(TEXT("WaitForSingleObject")); }
			else { Helpers::print_debug("FAILED stopping audio output.\n"); }
		}
		alreadyJamming = false;
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