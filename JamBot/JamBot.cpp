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

double sampleDiff[10000];
double sampleValue[10000], multiplier;
void CloseThread(int id);
void CloseAllThreads();
void ErrorHandler(LPTSTR lpszFunction);
const gchar *textInput;
GtkWidget *window, *lyricsEntry, *lyricsLabel, *intensityProgressBar, *freqLabel, *statusLabel;
GtkWidget *textEntry, *songSelectBox, *numChannelBox;
GList *songList = NULL;
string waveFilePath, lyricsPath;
string lyrics, csvFileName;
GtkTextBuffer *lyricsBuffer;
deque<GtkWidget*> sectionNameDetails;
deque<GtkWidget*> sectionTimeDetails;
ofstream masterCSV;
deque<string> csvList;
deque<int> channelList;
GtkListStore *liststore;
bool songSelectedFlag = false;
bool graphWaveFlag = false;
GtkWidget *drawArea, *sectionDialog, *valueDrawArea, *sectionWarningLabel;
bool songSectionFlag = false;
bool alreadyJamming = false;
bool waveSavedFlag = false;
int songListPosition = 0, songLength, graph_x, graph_y;
static int numberOfChannels;
GtkWidget *instrumentFrequency, *instrumentLoudness, *instrumentTempo;
int counter = 0;
cairo_t *crWave;

// Functions to run components in threads
DWORD WINAPI AudioInputThread(LPVOID lpParam) { inputChannelReader = InputChannelReader(); Helpers::print_debug("START audio input.\n"); inputChannelReader.start(songSelectedFlag); return 0; }
DWORD WINAPI WavGenThread(LPVOID lpParam) { wavmanipulation = WavManipulation(); Helpers::print_debug("START wav manip.\n"); wavmanipulation.start(csvFileName); return 0; }
DWORD WINAPI OptiAlgoThread(LPVOID lpParam) { optiAlgo = OptiAlgo(); Helpers::print_debug("START opti algo.\n"); optiAlgo.start(songSelectedFlag); return 0; }
DWORD WINAPI AudioOutputThread(LPVOID lpParam) { lightsTest = DMXOutput(); Helpers::print_debug("START audio output.\n"); lightsTest.start(); return 0; }


// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

void JamBot::updateLyrics(string text) {
	lyrics = text + "\n";
	gtk_label_set_text(GTK_LABEL(lyricsLabel), lyrics.c_str());
	gtk_widget_show_all(lyricsLabel);
}

static void changeProgressBar(double loudness, double freq)
{
	/*gtk_progress_bar_update(GTK_PROGRESS_BAR(intensityProgressBar), loudness);
	gtk_progress_bar_update(GTK_PROGRESS_BAR(freqProgressBar), freq);*/
}

static void setNumberOfChannel(GtkWidget *widget)
{
	numberOfChannels = gtk_combo_box_get_active(GTK_COMBO_BOX(widget)) + 1;
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
		/*setting up where the graph will be displayed*/
		cairo_t *crWave = gdk_cairo_create(drawArea->window);
		cairo_set_source_rgba(crWave, 1, 0.4, 0.3, 0.8);
		cairo_set_line_width(crWave, 1.3);
		double halfHeight = (graph_y / 2);
		/*setting the starting point of the graph*/
		cairo_move_to(crWave, 0.0, halfHeight);
		double interval = graph_x/10000.00;
		for (int i = 1; i < 10000; i++)
		{
			cairo_rel_line_to(crWave, interval, sampleDiff[i]*multiplier);
		}

		cairo_stroke(crWave);

		cairo_move_to(crWave, 0.0, halfHeight);
		cairo_set_source_rgba(crWave, 0, 0, 0, 1.0);
		cairo_rel_line_to(crWave, graph_x, 0);

		cairo_move_to(crWave, 0.0, halfHeight);
		cairo_rel_line_to(crWave, 0, halfHeight);

		cairo_move_to(crWave, 0.0, halfHeight);
		cairo_rel_line_to(crWave, 0, -1*halfHeight);

		/*move back to origin*/
		int tick;
		double x;
		int numOfTicks = floor(songLength / 5);
		cairo_move_to(crWave, 0.0, (graph_y/2));
		for (int i = 0; i < numOfTicks; i++) {
			x = (graph_x/numOfTicks)*i;
			/*determine the "tick" length based on 5's or 10's*/
			if (i % 2)
			{
				tick = 5;
			}
			else
			{
				tick = 10;
			}
			cairo_move_to(crWave, x, halfHeight);
			cairo_rel_line_to(crWave, 0, tick);
			cairo_rel_line_to(crWave, 0, (-2 * tick));
			cairo_move_to(crWave, x, halfHeight);
		}
		cairo_stroke(crWave);
		waveSavedFlag = true;
	}
}

static void graphValues()
{
	if (graphWaveFlag)
	{
		
	}

	/*cairo_t *cr = gdk_cairo_create(valueDrawArea->window);
	cairo_set_line_width(cr, 1.3);

	cairo_move_to(cr, 0.0, 100.0);
	cairo_set_source_rgba(cr, 0, 0, 0, 1.0);
	cairo_rel_line_to(cr, 500, 0);

	cairo_move_to(cr, 0.0, 100.0);
	cairo_rel_line_to(cr, 0, 100);

	cairo_move_to(cr, 0.0, 100.0);
	cairo_rel_line_to(cr, 0, -100.0);
	cairo_stroke(cr);

	cairo_t *crFreq = gdk_cairo_create(valueDrawArea->window);
	cairo_set_source_rgba(crFreq, 0.7, 0.7, 0.8, 0.8);
	cairo_set_line_width(cr, 1.3);

	cairo_move_to(crFreq, 0.0, 100.0);
	cairo_rel_line_to(crFreq, 230, 70);
	cairo_rel_line_to(crFreq, 50, -170);
	cairo_stroke(crFreq);*/
}

static void updateGraph()
{
	if (crWave != NULL)
	{
		cairo_restore(crWave);
		cairo_stroke(crWave);
		cairo_save(crWave);
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
			section.push_back(SongSection(name, strtod((char*)time, NULL)));
		}
		int position = lyricsPath.find_last_of('/\\');
		string fileName = lyricsPath.substr(position + 1);
		fileName = fileName.substr(0, fileName.size() - 4);

		bool submit = true;
		for (int i = 0; i < csvList.size(); i++)
		{
			if (csvList[i].compare(fileName) == 0)
			{
				submit = false;
			}
		}
		if (submit)
		{
			wavmanipulation.dataStore(fileName, section, waveFilePath, lyricsPath);

			masterCSV.open("CSV\\masterCSV.csv", ios_base::app);
			masterCSV << fileName + "\n";
			masterCSV.close();

			csvList.push_back(fileName);

			gtk_list_store_insert_with_values(liststore, NULL, -1, 0, "red", 1, (char*)fileName.c_str(), -1);
			gtk_dialog_response(GTK_DIALOG(sectionDialog), GTK_RESPONSE_CLOSE);

		}
		gtk_widget_show_all(songSelectBox);
	} 
	else
	{
		gtk_label_set_text(GTK_LABEL(sectionWarningLabel), "Include lyrics file (.txt) and an audio file (.wav)");
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

		if (!lyricsPath.empty() && !waveFilePath.empty()) {
			sectionWarningLabel = gtk_label_new("");
		}
		else
		{
			sectionWarningLabel = gtk_label_new("Include lyrics file (.txt) and an audio file (.wav)");
		}
		gtk_box_pack_start(GTK_BOX(GTK_DIALOG(sectionDialog)->vbox), sectionWarningLabel, false, false, 5);

		hbox = gtk_hbox_new(false, 0);
		addSectionButton = gtk_button_new_with_label("Add Section");
		g_signal_connect(GTK_OBJECT(addSectionButton), "clicked", G_CALLBACK(addNewSection), NULL);
		gtk_box_pack_start(GTK_BOX(hbox), addSectionButton, false, false, 5);

		submitButton = gtk_button_new_with_label("Submit");
		g_signal_connect(GTK_OBJECT(submitButton), "clicked", G_CALLBACK(submitSongSection), NULL, NULL);
		gtk_box_pack_start(GTK_BOX(hbox), submitButton, false, false, 5);

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
		SoundFileRead insound((waveFilePath).c_str());
		SoundHeader header = insound;
		
		int sampleAmount = header.getSamples();
		songLength = floor((double)insound.getSamples() / (double)insound.getSrate());
		/*getting how many samples we are going to take for this graph*/
		int sampleRate = sampleAmount / 10000;
		/*first sample*/
		int currentSample = sampleRate;
		/*getting the value of the beginning sample*/
		insound.gotoSample(0);
		sampleValue[0] = insound.getCurrentSampleDouble(0);
		sampleDiff[0] = 0.00;
		double max = 0.00;
		double min = 0.00;
		multiplier = 0.00;
		for (int i = 1; i < 10000; i++)
		{
			/*go to the next sample*/
			insound.gotoSample(currentSample);
			sampleValue[i] = insound.getCurrentSampleDouble(0);
			sampleDiff[i] = sampleValue[i] - sampleValue[i - 1];
			if (sampleDiff[i] > max)
			{
				max = sampleDiff[i];
			}
			else if (sampleDiff[i] < min)
			{
				min = sampleDiff[i];
			}
			currentSample += sampleRate;
		}

		if (max > abs(min))
		{
			multiplier = ceil(graph_y / 2) / max;
		}
		else
		{
			multiplier = ceil(graph_y / 2) / abs(min);
		}
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
	GtkWidget *lyricsDialog, *image;
	lyricsDialog = gtk_dialog_new_with_buttons("Nonmodal dialog", GTK_WINDOW(window), GTK_DIALOG_DESTROY_WITH_PARENT, NULL,
		NULL, NULL, NULL);
	gtk_window_set_decorated(GTK_WINDOW(lyricsDialog), false);
	gtk_window_fullscreen(GTK_WINDOW(lyricsDialog));

	lyricsLabel = gtk_label_new(lyrics.c_str());
	PangoFontDescription *font_desc;
	font_desc = pango_font_description_from_string("Ariel Bold 35");
	gtk_widget_modify_font(lyricsLabel, font_desc);
	pango_font_description_free(font_desc);

	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(lyricsDialog)->vbox), lyricsLabel, false, false, 5);

	gtk_widget_show_all(lyricsDialog);
}

static void displayLyrics(GtkWidget *widget, gpointer window)
{
	GtkWidget *lyricsDialog, *label;
	PangoFontDescription *font_desc;
	lyricsDialog = gtk_dialog_new_with_buttons("Lyrics Display", GTK_WINDOW(window), GTK_DIALOG_MODAL, NULL, NULL,
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
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(lyricsDialog)->vbox), lyricsEntry, false, false, 5);

	gtk_widget_show_all(lyricsDialog);
	gint response = gtk_dialog_run(GTK_DIALOG(lyricsDialog));
	if (response == GTK_RESPONSE_OK){
		g_print("The OK is pressed");
	}
	else {
		g_print("The cancel was pressed");
	}
	gtk_widget_destroy(lyricsDialog);
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
			gtk_label_set_text(GTK_LABEL(statusLabel), "Started with audio track");

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
		else
		{
			gtk_label_set_text(GTK_LABEL(statusLabel), "Started with live music");
			songSelectedFlag = false;
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

void JamBot::updateSongValues(float frequency, double loudness, double tempo)
{
	/*double loud = (int)((loudness / 7000) / 0.01) * 0.01;
	if (counter% 3 == 1)
	{
		gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(intensityProgressBar), loud);
	}
	if (counter == 10)
	{
		gtk_label_set_text(GTK_LABEL(freqLabel), to_string(frequency).c_str());
		counter = 0;
	}
	counter++;*/
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
	GtkWidget *window, *overAllWindowBox;
	GtkWidget *emersonButtonBox;
	GtkWidget *firstLayerBox, *lyricsBox, *songControlBox, *songInputBox, *songControlOverAllBox, *voiceProgressBox, *outputLyrics, *testButton, *songStatusBox;
	GtkWidget *secondLayerBox, *tabBox, *songProgressBox;
	GtkWidget *thirdLayerBox, *sectionBox;
	GtkWidget *playButton, *progressBar, *progressBarTest, *showModalDialog, *showNonmodalDialog, *fileSelectDialog, *emersonButton;
	GtkWidget *startJambot, *graphBox;
	GtkWidget *label;

	gtk_init(&argc, &argv);
	lyrics = "";
	/*=========================== Window ===========================*/
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	g_signal_connect(window, "delete-event", G_CALLBACK(delete_event), NULL);
	g_signal_connect(window, "destroy", G_CALLBACK(destroy), NULL);

	//gtk_container_set_border_width(GTK_CONTAINER(window), 150);
	gtk_window_set_title(GTK_WINDOW(window), "JamBot");

	g_signal_connect(window, "dontcallthis", G_CALLBACK(changeProgressBar), NULL);
	/*=========================== Widget boxes ===========================*/
	/*start of the window box*/
	overAllWindowBox = gtk_vbox_new(false, 0);

	/*emerson button box*/
	emersonButtonBox = gtk_hbox_new(false, 0);
	gtk_box_pack_start(GTK_BOX(overAllWindowBox), emersonButtonBox, false, false, 5);

	emersonButton = gtk_button_new_with_label("Emerson Button");
	//gtk_box_pack_start(GTK_BOX(emersonButtonBox), emersonButton, false, false, 5);
	g_signal_connect(GTK_OBJECT(emersonButton), "clicked", G_CALLBACK(startEmerson), window);

	/*first layer box, contains lyrics and play buttons*/
	firstLayerBox = gtk_hbox_new(false, 0);
	gtk_box_pack_start(GTK_BOX(overAllWindowBox), firstLayerBox, false, false, 5);

	/*lyrics box*/
	lyricsBox = gtk_hbox_new(false, 0);
	gtk_box_pack_start(GTK_BOX(firstLayerBox), lyricsBox, false, false, 5);
	
	/*song control over all box*/
	songControlOverAllBox= gtk_vbox_new(false, 0);
	gtk_box_pack_start(GTK_BOX(firstLayerBox), songControlOverAllBox, false, false, 5);

	/*song control box layer 1 AKA song control box*/
	songControlBox = gtk_hbox_new(false, 0);
	gtk_box_pack_start(GTK_BOX(songControlOverAllBox), songControlBox, false, false, 5);

	/*song control box layer 2 AKA song status box*/
	songStatusBox = gtk_hbox_new(false, 0);
	gtk_box_pack_start(GTK_BOX(songControlOverAllBox), songStatusBox, false, false, 5);

	/*second layer box, contains song value tab box*/
	secondLayerBox = gtk_hbox_new(false, 0);
	gtk_box_pack_start(GTK_BOX(overAllWindowBox), secondLayerBox, false, false, 5);

	/*tab box*/
	tabBox = gtk_vbox_new(false, 0);
	gtk_box_pack_start(GTK_BOX(secondLayerBox), tabBox, false, false, 5);

	/*third layer box, contains section buttons and graph*/
	thirdLayerBox = gtk_vbox_new(false, 0);
	gtk_box_pack_start(GTK_BOX(overAllWindowBox), thirdLayerBox, false, false, 5);

	/*section box*/
	sectionBox = gtk_hbox_new(false, 0);
	gtk_box_pack_start(GTK_BOX(thirdLayerBox), sectionBox, false, false, 5);

	/*graph box, this containts the combo box part*/
	graphBox = gtk_vbox_new(false, 0);
	gtk_box_pack_start(GTK_BOX(thirdLayerBox), graphBox, false, false, 5);

	/*=========================== third layer Part ===========================*/
	GtkWidget *sectionLabelName, *sectionLabelTime, *addSectionButton, *submitSectionButton, *tempEntry;

	/*sectionButtonBox*/
	addSectionButton = gtk_button_new_with_label("Section");
	g_signal_connect(GTK_OBJECT(addSectionButton), "clicked", G_CALLBACK(displaySectionModal), NULL, NULL);
	gtk_box_pack_start(GTK_BOX(sectionBox), addSectionButton, false, false, 5);

	fileSelectDialog = gtk_button_new_with_label("Select Audio File");
	g_signal_connect(GTK_OBJECT(fileSelectDialog), "clicked", G_CALLBACK(selectWaveFile), NULL, NULL);
	gtk_box_pack_start(GTK_BOX(sectionBox), fileSelectDialog, false, false, 5);

	/*combo box part*/
	songSelectBox = gtk_combo_box_new();
	//g_signal_connect(GTK_OBJECT(songSelectBox), "move-active", G_CALLBACK(testFunction), NULL);

	GtkCellRenderer *column;
	//gtk_init(&argc, &argv);

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

	column = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(songSelectBox), column, TRUE);
	gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(songSelectBox), column, "cell-background", 0, "text", 1, NULL);
	gtk_combo_box_set_active(GTK_COMBO_BOX(songSelectBox), 0);
	gtk_box_pack_start(GTK_BOX(sectionBox), songSelectBox, false, false, 5);

	/*configure draw area*/
	drawArea = gtk_drawing_area_new();
	gtk_widget_set_size_request(GTK_WIDGET(drawArea), 550, 300);
	graph_x = 500;
	graph_y = 300;
	gtk_window_set_resizable(GTK_WINDOW(drawArea), FALSE);
	gtk_box_pack_start(GTK_BOX(graphBox), drawArea, true, true, 0);

	g_signal_connect(drawArea, "expose-event", G_CALLBACK(graphWave), NULL);

	/*=========================== Second Layer Part ===========================*/
	GtkWidget * tabs, *label_voice, *label_song, *tempLabel, *vbox;
	tabs = gtk_notebook_new();
	label_voice = gtk_label_new("Voice");
	label_song = gtk_label_new("Instrument");

	songProgressBox = gtk_hbox_new(false, 0);
	voiceProgressBox = gtk_hbox_new(false, 0);

	gtk_notebook_append_page(GTK_NOTEBOOK(tabs), songProgressBox, label_song);
	gtk_notebook_append_page(GTK_NOTEBOOK(tabs), voiceProgressBox, label_voice);
	
	/*Instrument Tab*/
	vbox = gtk_vbox_new(false, 0);
	/*label for loudness meter*/
	tempLabel = gtk_label_new("Loudness Meter (db):");
	gtk_box_pack_start(GTK_BOX(vbox), tempLabel, false, false, 5);
	/*label for frequency*/
	tempLabel = gtk_label_new("Frequency (Hz):");
	gtk_box_pack_start(GTK_BOX(vbox), tempLabel, false, false, 5);
	gtk_box_pack_start(GTK_BOX(songProgressBox), vbox, false, false, 5);

	/*getting the second vbox*/
	vbox = gtk_vbox_new(false, 0);
	/*intensity bar*/
	intensityProgressBar = gtk_progress_bar_new();
	//gtk_widget_set_size_request(GTK_WIDGET(intensityProgressBar), 100, 20);
	gtk_box_pack_start(GTK_BOX(vbox), intensityProgressBar, false, false, 5);
	
	/*frequency label*/
	freqLabel = gtk_label_new("");
	gtk_box_pack_start(GTK_BOX(vbox), freqLabel, false, false, 5);
	gtk_box_pack_start(GTK_BOX(songProgressBox), vbox, false, false, 5);

	/*Voice Tab*/
	vbox = gtk_vbox_new(false, 0);
	/*label for loudness meter*/
	tempLabel = gtk_label_new("Loudness Meter (db):");
	gtk_box_pack_start(GTK_BOX(vbox), tempLabel, false, false, 5);
	/*label for frequency*/
	tempLabel = gtk_label_new("Frequency (Hz):");
	gtk_box_pack_start(GTK_BOX(vbox), tempLabel, false, false, 5);
	gtk_box_pack_start(GTK_BOX(voiceProgressBox), vbox, false, false, 5);

	/*getting the second vbox*/
	vbox = gtk_vbox_new(false, 0);
	/*intensity bar*/
	intensityProgressBar = gtk_progress_bar_new();
	//gtk_widget_set_size_request(GTK_WIDGET(intensityProgressBar), 100, 20);
	gtk_box_pack_start(GTK_BOX(vbox), intensityProgressBar, false, false, 5);

	/*frequency label*/
	freqLabel = gtk_label_new("");
	gtk_box_pack_start(GTK_BOX(vbox), freqLabel, false, false, 5);
	gtk_box_pack_start(GTK_BOX(voiceProgressBox), vbox, false, false, 5);


	gtk_widget_set_size_request(GTK_WIDGET(tabs), 300, 100);
	gtk_window_set_resizable(GTK_WINDOW(tabs), FALSE);
	//gtk_box_pack_start(GTK_BOX(tabs), songProgressBox, false, false, 5);
	gtk_box_pack_start(GTK_BOX(tabBox), tabs, false, false, 5);

	/*========================================== First Layer Part ==========================================*/
	/*play button*/
	playButton = gtk_button_new_with_label("Play");
	gtk_box_pack_start(GTK_BOX(songControlBox), playButton, false, false, 5);
	g_signal_connect(GTK_OBJECT(playButton), "clicked", G_CALLBACK(startJamming), window);
	gtk_widget_show(playButton);

	/*pause button*/
	playButton = gtk_button_new_with_label("Stop");
	gtk_box_pack_start(GTK_BOX(songControlBox), playButton, false, false, 5);
	g_signal_connect(GTK_OBJECT(playButton), "clicked", G_CALLBACK(CloseAllThreads), NULL);
	gtk_widget_show(playButton);

	/*test button*/
	testButton = gtk_button_new_with_label("Test");
	//gtk_box_pack_start(GTK_BOX(songControlBox), testButton, false, false, 5);
	g_signal_connect(GTK_OBJECT(testButton), "clicked", G_CALLBACK(changeProgressBar), NULL);
	gtk_widget_show(testButton);

	/*number of channel combo box*/
	numChannelBox = gtk_combo_box_new();
	GtkCellRenderer *column2;
	//gtk_init(&argc, &argv);

	GtkListStore *channelListStore = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_STRING);
	
	gtk_list_store_insert_with_values(channelListStore, NULL, 0, 0, "red", 1, "Single Channel", -1);
	channelList.push_back(1);

	gtk_list_store_insert_with_values(channelListStore, NULL, 1, 0, "red", 1, "Dual Channel", -1);
	channelList.push_back(2);
	
	numChannelBox = gtk_combo_box_new_with_model(GTK_TREE_MODEL(channelListStore));
	g_signal_connect(GTK_OBJECT(numChannelBox), "changed", G_CALLBACK(setNumberOfChannel), NULL);

	column2 = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(numChannelBox), column2, TRUE);
	gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(numChannelBox), column2, "cell-background", 0, "text", 1, NULL);
	gtk_combo_box_set_active(GTK_COMBO_BOX(numChannelBox), 0);
	gtk_box_pack_start(GTK_BOX(songControlBox), numChannelBox, false, false, 5);

	/*status label*/
	statusLabel = gtk_label_new("Idle");
	gtk_box_pack_start(GTK_BOX(songStatusBox), statusLabel, false, false, 5);

	/*select lyrics*/
	fileSelectDialog = gtk_button_new_with_label("Select Lyrics");
	gtk_box_pack_start(GTK_BOX(lyricsBox), fileSelectDialog, false, false, 5);
	g_signal_connect(GTK_OBJECT(fileSelectDialog), "clicked", G_CALLBACK(selectLyrics), window);

	/*display lyrics*/
	outputLyrics = gtk_button_new_with_label("Display Lyrics");
	g_signal_connect(GTK_OBJECT(outputLyrics), "clicked", G_CALLBACK(displayLyricsNonmodal), window);
	gtk_box_pack_start(GTK_BOX(lyricsBox), outputLyrics, false, false, 5);

	/*================================== the rest =====================================*/
	gtk_container_add(GTK_CONTAINER(window), overAllWindowBox);

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
		gtk_label_set_text(GTK_LABEL(statusLabel), "Idle");
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