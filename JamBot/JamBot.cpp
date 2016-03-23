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
#include <array>

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
GtkWidget *window, *lyricsEntry, *lyricsLabel, *intensityProgBarInstru, *intensityProgBarVoice, *freqLabelInstru, *freqLabelVoice, *statusLabel, *globalWindow;
GtkWidget *freeplay, *concert;	//For use in selecting run-mode
GtkWidget *freqHighColours, *freqLowColours, *tempoColours;	//For use in selecting colours
GtkWidget *textEntry, *songSelectBox, *numChannelBox;
GList *songList = NULL;
string waveFilePath, lyricsPath;
string lyrics, csvFileName;
GtkTextBuffer *lyricsBuffer;
deque<GtkWidget*> sectionNameDetails;
deque<GtkWidget*> sectionTimeDetails;
deque<GtkWidget*> sectionStrobeDetails;
ofstream masterCSV;
deque<string> csvList;
//deque<int> channelList;
GtkListStore *liststore;
bool songSelectedFlag = false;
bool graphWaveFlag = false;
GtkWidget *drawArea, *sectionDialog, *valueDrawArea, *sectionWarningLabel;
bool songSectionFlag = false;
bool alreadyJamming = false;
bool waveSavedFlag = false;
bool onDefaultTab = false;
int songListPosition = 0, songLength, graph_x, graph_y;
int numberOfChannels = 1;
GtkWidget *instrumentFrequency, *instrumentLoudness, *instrumentTempo;
int counterInstru = 0;
int counterVoice = 0;
cairo_t *crWave;
bool doneReading = false;
bool autoStrobe = false;
bool concertMode = false;
array<OutParams, 3> colourScheme;

// Functions to run components in threads
DWORD WINAPI AudioInputThread(LPVOID lpParam) { inputChannelReader = InputChannelReader(); Helpers::print_debug("START audio input.\n"); inputChannelReader.start(songSelectedFlag, numberOfChannels); return 0; }
DWORD WINAPI WavGenThread(LPVOID lpParam) { wavmanipulation = WavManipulation(); Helpers::print_debug("START wav manip.\n"); wavmanipulation.start(csvFileName); return 0; }
DWORD WINAPI OptiAlgoThread(LPVOID lpParam) { optiAlgo = OptiAlgo(); Helpers::print_debug("START opti algo.\n"); optiAlgo.start(concertMode, colourScheme, autoStrobe); return 0; }
DWORD WINAPI AudioOutputThread(LPVOID lpParam) { lightsTest = DMXOutput(); Helpers::print_debug("START audio output.\n"); lightsTest.start(); return 0; }


// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);
#include "wtypes.h"
#include <iostream>
using namespace std;

// Get the horizontal and vertical screen sizes in pixel
// Depricated
void GetDesktopResolution(int& horizontal, int& vertical)
{
	RECT desktop;
	// Get a handle to the desktop window
	const HWND hDesktop = GetDesktopWindow();
	// Get the size of screen to the variable desktop
	GetWindowRect(hDesktop, &desktop);
	// The top left corner will have coordinates (0,0)
	// and the bottom right corner will have coordinates
	// (horizontal, vertical)
	horizontal = desktop.right;
	vertical = desktop.bottom;
}

void JamBot::updateLyrics(string text) {
	int font_size = 30;
	int max_size = 0;
	string font = "BoostLightSSK Regular ";
	string temp = "";
	//int horizontal = 0;
	//int vertical = 0;
	//GetDesktopResolution(horizontal, vertical);
	lyrics = text + "\n";

	//Depricated, using JUSTIFY_FILL instead
	//temp = lyrics;
	//istringstream iss(text); 
	//vector<string> tokens;
	//std::string delimiter = "\n";

	//size_t pos = 0;
	//std::string token;
	//while ((pos = temp.find(delimiter)) != std::string::npos) {
	//	token = temp.substr(0, pos);
	//	tokens.push_back(token);
	//	//std::cout << token << std::endl;
	//	temp.erase(0, pos + delimiter.length());
	//}
	//tokens.push_back(token);
	////std::cout << lyrics << std::endl;
	//for (int i = 0; i < tokens.size(); i++){
	//	if (tokens[i].length() > max_size)
	//		max_size = tokens[i].length();
	//}
	//if (max_size >= 50){
	//	font_size = max_size/1.5;
	//}

	PangoFontDescription *font_desc = pango_font_description_new();
	font = font + to_string(font_size);
	font_desc = pango_font_description_from_string(font.c_str());
	gtk_widget_modify_font(lyricsLabel, font_desc);
	//gtk_label_set_justify(GTK_LABEL(lyricsLabel), GTK_JUSTIFY_FILL);
	gtk_label_set_text(GTK_LABEL(lyricsLabel), lyrics.c_str());
	gtk_widget_show_all(lyricsLabel);
}

void JamBot::initialReading(bool done)
{
	doneReading = done;
}

static void changeProgressBar(double loudness, double freq)
{
	/*gtk_progress_bar_update(GTK_PROGRESS_BAR(intensityProgressBar), loudness);
	gtk_progress_bar_update(GTK_PROGRESS_BAR(freqProgressBar), freq);*/
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
		sectionStrobeDetails.clear();
		gtk_widget_destroy(dialog);
	}
}

static void graphWave() {

	/*if (graphWaveFlag)
	{
		/*setting up where the graph will be displayed*/
/*		gdk_window_clear_area(drawArea->window, 0, 0, 550, graph_y);
		cairo_t *crWave = gdk_cairo_create(drawArea->window);
		cairo_set_source_rgba(crWave, 1, 0.4, 0.3, 0.8);
		cairo_set_line_width(crWave, 1.3);
		double halfHeight = (graph_y / 2);
		/*setting the starting point of the graph*/
/*		cairo_move_to(crWave, 0.0, halfHeight);
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
/*		int tick;
		double x;
		int numOfTicks = floor(songLength / 5);
		cairo_move_to(crWave, 0.0, (graph_y/2));
		for (int i = 0; i < numOfTicks; i++) {
			x = (graph_x/numOfTicks)*i;
			/*determine the "tick" length based on 5's or 10's*/
/*			if (i % 2)
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
	}*/
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
	GtkWidget *hbox, *tempEntry, *tempCheck;

	hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);

	tempEntry = gtk_entry_new();
	sectionNameDetails.push_back(tempEntry);
	gtk_box_pack_start(GTK_BOX(hbox), tempEntry, true, true, 5);

	tempEntry = gtk_entry_new();
	sectionTimeDetails.push_back(tempEntry);
	gtk_box_pack_start(GTK_BOX(hbox), tempEntry, true, true, 5);

	tempCheck = gtk_check_button_new();
	sectionStrobeDetails.push_back(tempCheck);
	gtk_box_pack_start(GTK_BOX(hbox), tempCheck, true, true, 7);

	gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(sectionDialog))), hbox, true, true, 5);

	gtk_widget_show_all(sectionDialog);
}

static void submitSongSection() {
	GtkWidget *sectionTime, *sectionName;
	const gchar *name, *time;
	bool strobe;
	GtkCellRenderer *column;

	if (!lyricsPath.empty() && !waveFilePath.empty()) {
		vector<SongSection> section = vector<SongSection>();

		for (int i = 0; i < sectionNameDetails.size(); i++) {
			name = gtk_entry_get_text(GTK_ENTRY(sectionNameDetails[i]));
			time = gtk_entry_get_text(GTK_ENTRY(sectionTimeDetails[i]));
			strobe = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(sectionStrobeDetails[i]));
			section.push_back(SongSection(name, strtod((char*)time, NULL), strobe)); 
		}
		int position = lyricsPath.find_last_of('/\\');
		string fileName = lyricsPath.substr(position + 1);
		fileName = fileName.substr(0, fileName.size() - 4);

		wavmanipulation.dataStore(fileName, section, waveFilePath, lyricsPath);


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
			masterCSV.open("CSV\\masterCSV.csv", ios_base::app);
			masterCSV << fileName + "\n";
			masterCSV.close();

			csvList.push_back(fileName);

			gtk_list_store_insert_with_values(liststore, NULL, -1, 0, NULL, 1, (char*)fileName.c_str(), -1);
		}
		gtk_widget_show_all(songSelectBox);
		gtk_dialog_response(GTK_DIALOG(sectionDialog), GTK_RESPONSE_CLOSE);
	} 
	else
	{
		gtk_label_set_text(GTK_LABEL(sectionWarningLabel), "Include lyrics file (.txt) and an audio file (.wav)");
		gtk_widget_show_all(songSelectBox);
	}
}

static void selectWaveFile(GtkWidget *widget) {

	GtkWidget *dialog;
	static gint i = 1;

	dialog = gtk_file_chooser_dialog_new("Choose a file", GTK_WINDOW(window), GTK_FILE_CHOOSER_ACTION_OPEN, GTK_STOCK_OK,
		GTK_RESPONSE_OK, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, NULL);
	gtk_widget_show_all(dialog);
	gint resp = gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_window_set_resizable(GTK_WINDOW(window), false);
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

static void displaySectionModal(GtkWidget *widget, gint resp, gpointer *data)
{
	if (!songSectionFlag)
	{
		gint position = gtk_combo_box_get_active(GTK_COMBO_BOX(songSelectBox));
		csvFileName = csvList[position] + ".csv";

		string wav, lyric;
		vector<string> name;
		vector<string> time;
		vector<bool> strobe;

		GtkWidget *hbox, *label, *tempEntry, *addSectionButton, *submitButton, *tempCheck, *fileSelectDialog;
		sectionDialog = gtk_dialog_new_with_buttons("Nonmodal dialog", GTK_WINDOW(window), GTK_DIALOG_DESTROY_WITH_PARENT, NULL,
			NULL, NULL, NULL);
		g_signal_connect(sectionDialog, "response", G_CALLBACK(dialog_result), NULL);

		if ((!lyricsPath.empty() && !waveFilePath.empty()) || position > 0) {
			sectionWarningLabel = gtk_label_new("");
		}
		else
		{
			sectionWarningLabel = gtk_label_new("Include lyrics file (.txt) and an audio file (.wav)");
		}
		gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(sectionDialog))), sectionWarningLabel, true, true, 5);

		hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
		gtk_box_set_homogeneous(GTK_BOX(hbox), true);
		fileSelectDialog = gtk_button_new_with_label("Select Audio File");
		g_signal_connect(GTK_DIALOG(fileSelectDialog), "clicked", G_CALLBACK(selectWaveFile), NULL, NULL);
		gtk_box_pack_start(GTK_BOX(hbox), fileSelectDialog, true, true, 5);

		/*select lyrics*/
		fileSelectDialog = gtk_button_new_with_label("Select Lyrics");
		gtk_box_pack_start(GTK_BOX(hbox), fileSelectDialog, true, true, 5);
		g_signal_connect(GTK_DIALOG(fileSelectDialog), "clicked", G_CALLBACK(selectLyrics), window);
		gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(sectionDialog))), hbox, true, true, 5);

		hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
		gtk_box_set_homogeneous(GTK_BOX(hbox), true);
		addSectionButton = gtk_button_new_with_label("Add Section");
		g_signal_connect(GTK_BUTTON(addSectionButton), "clicked", G_CALLBACK(addNewSection), NULL);
		gtk_box_pack_start(GTK_BOX(hbox), addSectionButton, true, true, 5);

		submitButton = gtk_button_new_with_label("Apply");
		g_signal_connect(GTK_BUTTON(submitButton), "clicked", G_CALLBACK(submitSongSection), NULL, NULL);
		gtk_box_pack_start(GTK_BOX(hbox), submitButton, true, true, 5);
		gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(sectionDialog))), hbox, true, true, 5);


		hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
		label = gtk_label_new("Section Name");
		gtk_box_pack_start(GTK_BOX(hbox), label, true, true, 5);
		label = gtk_label_new("Section Time");
		gtk_box_pack_start(GTK_BOX(hbox), label, true, true, 80);
		label = gtk_label_new("Strobe");
		gtk_box_pack_start(GTK_BOX(hbox), label, true, true, 5);
		gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(sectionDialog))), hbox, true, true, 5);

		if (position > 0)
		{
			// Read values into text boxes
			WavManipulation::readCSV(csvFileName, wav, lyric, name, time, strobe);

			// Set Wave/Lyric path for overwrite
			lyricsPath = lyric;
			waveFilePath = wav;

			for (int i = 0; i < name.size(); i++)
			{
				hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);

				tempEntry = gtk_entry_new();
				sectionNameDetails.push_back(tempEntry);
				gtk_entry_set_text(GTK_ENTRY(tempEntry), name[i].c_str());
				gtk_box_pack_start(GTK_BOX(hbox), tempEntry, true, true, 5);

				tempEntry = gtk_entry_new();
				sectionTimeDetails.push_back(tempEntry);
				gtk_entry_set_text(GTK_ENTRY(tempEntry), time[i].c_str());
				gtk_box_pack_start(GTK_BOX(hbox), tempEntry, true, true, 5);

				tempCheck = gtk_check_button_new();
				gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tempCheck), strobe[i]);
				sectionStrobeDetails.push_back(tempCheck);
				gtk_box_pack_start(GTK_BOX(hbox), tempCheck, true, true, 7);

				gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(sectionDialog))), hbox, true, true, 5);
			}
		}

		gtk_widget_show_all(sectionDialog);

		songSectionFlag = true;
	}
}

// NOT USED HOMBRE
static void graphWaveFromCSV()
{
	gint position = gtk_combo_box_get_active(GTK_COMBO_BOX(songSelectBox));
	string fileName = csvList[position] + ".csv";
	if ((position > 0) && (fileName.compare("None.csv") != 0))
	{
		ifstream file("CSV\\"+fileName);
		string line, text;

		if (file.is_open())
		{
			getline(file, line);
			getline(file, line);
			int position = line.find_first_of(",");
			waveFilePath = line.substr(0, position);
			file.close();

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
		
	}
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
	font_desc = pango_font_description_from_string("BoostLightSSK Regular 30");
	gtk_widget_modify_font(lyricsLabel, font_desc);
	pango_font_description_free(font_desc);

	gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(lyricsDialog))), lyricsLabel, false, false, 5);

	gtk_label_set_justify(GTK_LABEL(lyricsLabel), GTK_JUSTIFY_LEFT);

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
	gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(lyricsDialog))), lyricsEntry, false, false, 5);

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

void updateGlobals()
{
	gint position = gtk_combo_box_get_active(GTK_COMBO_BOX(songSelectBox));
	// Number of channels to use
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(concert)))
		numberOfChannels = 2;
	else
		numberOfChannels = 1;

	// Concert mode enabled?
	concertMode = numberOfChannels == 2 && (position > 0 || csvFileName.compare("None.csv") != 0);

	// Get user selected colours
	colourScheme[0] = (OutParams)gtk_combo_box_get_active(GTK_COMBO_BOX(freqLowColours));	//Low freq
	colourScheme[1] = (OutParams)gtk_combo_box_get_active(GTK_COMBO_BOX(freqHighColours));	//High freq
	colourScheme[2] = (OutParams)gtk_combo_box_get_active(GTK_COMBO_BOX(tempoColours));	//High tempo
}

static void startJamming(GtkWidget *button) {
	if (!alreadyJamming) {

		// Update global variables to be sent to subsections
		updateGlobals();

		if (concertMode)
		{
			gtk_label_set_text(GTK_LABEL(statusLabel), "Concert Mode: Initiated");

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

			while (!doneReading) {}	//Wait while .wav not read
		}
		else
		{
			gtk_label_set_text(GTK_LABEL(statusLabel), "Freeplay: Initiated");
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
		onDefaultTab = true;
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

static gboolean updateProgressBar(gpointer loudness)
{
	/*int i = 0;
	//gdouble *j = loudness;
	int j = (rand() % 100);
	double loud = j / 100.00;
	if (counterInstru % 10 == 1)
	{
		gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(intensityProgBarInstru), 1 * loud);
		Sleep(100);
		gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(intensityProgBarInstru), 0.9*loud);
		Sleep(100);
		gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(intensityProgBarInstru), 0.8*loud);
		Sleep(100);
		gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(intensityProgBarInstru), 0.7*loud);
		Sleep(100);
		gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(intensityProgBarInstru), 0.6*loud);
		Sleep(100);
		gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(intensityProgBarInstru), 0.5*loud);
		Sleep(100);
		gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(intensityProgBarInstru), 0.4*loud);
		Sleep(100);
		gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(intensityProgBarInstru), 0.3*loud);
		Sleep(100);
		gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(intensityProgBarInstru), 0.2*loud);

	}

	counterInstru++;
	return G_SOURCE_REMOVE;*/
}

void JamBot::updateSongValues(float frequency, double loudness, double tempo)
{
	/*double* j = &loudness;
	if (onDefaultTab)
	{
		gdk_threads_add_idle((GSourceFunc) updateProgressBar, j);
		onDefaultTab = false;
	}
	/*
	double loud = (loudness / 2000.00);
	if (counterInstru % 5 == 1)
	{
		gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(intensityProgBarInstru), 1 * loud);
		Sleep(400);
		gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(intensityProgBarInstru), 0.66*loud);
		Sleep(25);
		gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(intensityProgBarInstru), 0.33*loud);
		Sleep(25);
		gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(intensityProgBarInstru), 0.15*loud);
		Sleep(25);
		gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(intensityProgBarInstru), 0.10*loud);
		Sleep(25);
		gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(intensityProgBarInstru), 0.08*loud);
	}

	if (counterInstru == 10)
	{
		gtk_label_set_text(GTK_LABEL(freqLabelInstru), to_string(frequency).c_str());
		counterInstru = 0;
	}
	counterInstru++;*/
}

void JamBot::updateVoiceValues(float frequency, double loudness, double tempo)
{
	//double loud = (loudness / 5000.00);
	//if (counterVoice % 5 == 1)
	//{
	//	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(intensityProgBarVoice), 1 * loud);
	//	Sleep(400);
	//	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(intensityProgBarVoice), 0.66*loud);
	//	Sleep(25);
	//	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(intensityProgBarVoice), 0.33*loud);
	//	Sleep(25);
	//	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(intensityProgBarVoice), 0.15*loud);
	//	Sleep(25);
	//	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(intensityProgBarVoice), 0.10*loud);
	//	Sleep(25);
	//	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(intensityProgBarVoice), 0.08*loud);

	//}

	//if (counterVoice == 10)
	//{
	//	gtk_label_set_text(GTK_LABEL(freqLabelVoice), to_string(frequency).c_str());
	//	counterVoice = 0;
	//}
	//counterVoice++;
}


static void tabSwitchPage()
{
	onDefaultTab = !onDefaultTab;
	gtk_label_set_text(GTK_LABEL(freqLabelInstru), "");
	gtk_label_set_text(GTK_LABEL(freqLabelVoice), "");
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

static void stopUpdating()
{
	/*clear out the lyrics*/
	lyrics.clear();
	gtk_label_set_text(GTK_LABEL(lyricsLabel), lyrics.c_str());
	/*dumb the data from InputChannelReader*/
	gtk_label_set_text(GTK_LABEL(freqLabelInstru), "");

}

int gtkStart(int argc, char* argv[])
{
	GtkWidget *window, *overAllWindowBox;
	GtkWidget *emersonButtonBox;
	GtkWidget *lightColourControlBox, *colourControlDropdownLayer, *freqControlBox, *tempoControlBox, *secondLayerBox, *tempLabel, *freqControlFrame, *tempoControlFrame, *radioButtonFrame;
	GtkWidget *firstLayerBox, *lyricsBox, *songControlBox, *songInputBox, *songControlOverAllBox, *voiceProgressBox, *outputLyrics, *testButton, *songStatusBox;
	GtkWidget *freqBox, *tempoBox, *freqHighLabelCol, *freqLowLabelCol, *tempoLabelCol;
	GtkWidget *tabBox, *songProgressBox;
	GtkWidget *thirdLayerBox, *sectionBox;
	GtkWidget *playButton, *progressBar, *progressBarTest, *showModalDialog, *showNonmodalDialog, *emersonButton;
	GtkWidget *startJambot, *graphBox;
	GtkWidget *label, *freqLabel, *tempoLabel, *freqHighListLabel, *freqLowListLabel, *tempoListLabel;

	/*profit from GTK dank libs effects*/
	gtk_init(&argc, &argv);
	lyrics = "";
	/*=========================== Window ===========================*/
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_resizable(GTK_WINDOW(window), false);
	globalWindow = gtk_button_new();
	g_signal_connect(window, "delete-event", G_CALLBACK(delete_event), NULL);
	g_signal_connect(window, "destroy", G_CALLBACK(destroy), NULL);

	//gtk_container_set_border_width(GTK_CONTAINER(window), 150);
	gtk_window_set_title(GTK_WINDOW(window), "JamBot");

	g_signal_connect(window, "dontcallthis", G_CALLBACK(changeProgressBar), NULL);
	g_signal_connect(globalWindow, "child-finished", G_CALLBACK(testFunction), NULL);
	/*=========================== Widget boxes ===========================*/
	/*start of the window box*/
	overAllWindowBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	gtk_widget_set_size_request(overAllWindowBox, 400, 400);
	/*song control box layer 2 AKA song status box*/
	songStatusBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	gtk_box_pack_start(GTK_BOX(overAllWindowBox), songStatusBox, false, false, 5);

	/*first layer box*/
	firstLayerBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	gtk_box_pack_start(GTK_BOX(overAllWindowBox), firstLayerBox, false, false, 5);
	gtk_widget_set_size_request(firstLayerBox, 400, 100);


	/*radio buttons for number of channels*/
	radioButtonFrame = gtk_frame_new("Run Mode");
	gtk_box_pack_start(GTK_BOX(firstLayerBox), radioButtonFrame, false, false, 5);
	numChannelBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

	/*dropdowns for tempo colour changer*/
	lightColourControlBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	gtk_container_set_border_width(GTK_CONTAINER(firstLayerBox), 0);
	gtk_box_pack_start(GTK_BOX(firstLayerBox), lightColourControlBox, false, false, 5);

	/*dropdown box for colour changer (control)*/
	colourControlDropdownLayer = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	gtk_box_pack_start(GTK_BOX(lightColourControlBox), colourControlDropdownLayer, false, false, 5);

	/*box for frequency colour control*/

	freqControlBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	gtk_box_pack_start(GTK_BOX(colourControlDropdownLayer), freqControlBox, false, false, 5);

	/*box for tempo colour control*/
	tempoControlBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	gtk_box_pack_start(GTK_BOX(colourControlDropdownLayer), tempoControlBox, false, false, 5);


	/*second layer box, contains song value tab box*/
	secondLayerBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	gtk_box_pack_start(GTK_BOX(overAllWindowBox), secondLayerBox, true, true, 5);

	/*tab box*/
	tabBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	gtk_box_pack_start(GTK_BOX(secondLayerBox), tabBox, true, true, 5);

	/*third layer box, contains section buttons and graph*/
	thirdLayerBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	gtk_box_pack_start(GTK_BOX(overAllWindowBox), thirdLayerBox, true, true, 5);

	/*lyrics box*/
	lyricsBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	gtk_box_pack_start(GTK_BOX(overAllWindowBox), lyricsBox, true, true, 5);

	/*song control box layer 1 AKA song control box*/
	songControlBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	gtk_box_pack_start(GTK_BOX(overAllWindowBox), songControlBox, true, true, 5);

	/*section box*/
	sectionBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	gtk_box_pack_start(GTK_BOX(thirdLayerBox), sectionBox, true, true, 5);

	/*graph box, this containts the combo box part*/
	graphBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	gtk_box_pack_start(GTK_BOX(thirdLayerBox), graphBox, true, true, 5);

	/*=========================== third layer Part ===========================*/
	GtkWidget *sectionLabelName, *sectionLabelTime, *addSectionButton, *submitSectionButton, *tempEntry, *songBox;

	//fileSelectDialog = gtk_button_new_with_label("Select Audio File");
	//g_signal_connect(GTK_DIALOG(fileSelectDialog), "clicked", G_CALLBACK(selectWaveFile), NULL, NULL);
	//gtk_box_pack_start(GTK_BOX(sectionBox), fileSelectDialog, true, true, 5);

	///*select lyrics*/
	//fileSelectDialog = gtk_button_new_with_label("Select Lyrics");
	//gtk_box_pack_start(GTK_BOX(sectionBox), fileSelectDialog, true, true, 5);
	//g_signal_connect(GTK_DIALOG(fileSelectDialog), "clicked", G_CALLBACK(selectLyrics), window);

	/*sectionButtonBox*/
	addSectionButton = gtk_button_new_with_label("Configure");
	g_signal_connect(GTK_BUTTON(addSectionButton), "clicked", G_CALLBACK(displaySectionModal), NULL, NULL);
	gtk_box_pack_start(GTK_BOX(sectionBox), addSectionButton, true, true, 5);

	/*combo box part*/
	songSelectBox = gtk_combo_box_new();

	GtkCellRenderer *column;
	//gtk_init(&argc, &argv);

	liststore = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_STRING);

	//songBox = gtk_combo_box_new_with_model(liststore);

	ifstream file("CSV\\masterCSV.csv");
	string line, text;

	if (!file)
	{
		ofstream ofile("CSV\\masterCSV.csv");
	}
	ifstream ifile("CSV\\masterCSV.csv");
	if (ifile.is_open())
	{
		gtk_list_store_insert_with_values(liststore, NULL, songListPosition, 0, NULL, 1, "None", -1);
		csvList.push_back("None");
		songListPosition++;

		while (getline(file, line))
		{
			gtk_list_store_insert_with_values(liststore, NULL, songListPosition, 0, NULL, 1, (char*)line.c_str(), -1);
			csvList.push_back(line);
			songListPosition++;
		}
		file.close();
	}

	songSelectBox = gtk_combo_box_new_with_model(GTK_TREE_MODEL(liststore));

	//g_signal_connect(GTK_OBJECT(songSelectBox), "changed", G_CALLBACK(graphWaveFromCSV), NULL);
	column = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(songSelectBox), column, TRUE);
	gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(songSelectBox), column, "cell-background", 0, "text", 1, NULL);
	gtk_combo_box_set_active(GTK_COMBO_BOX(songSelectBox), 0);
	gtk_box_pack_start(GTK_BOX(sectionBox), songSelectBox, false, false, 5);

	/*configure draw area*/
	/*drawArea = gtk_drawing_area_new();
	gtk_widget_set_size_request(GTK_WIDGET(drawArea), 550, 200);
	graph_x = 500;
	graph_y = 200;
	gtk_window_set_resizable(GTK_WINDOW(drawArea), FALSE);
	gtk_box_pack_start(GTK_BOX(graphBox), drawArea, true, true, 0);*/

	//g_signal_connect(drawArea, "expose-event", G_CALLBACK(graphWave), NULL);

	/*=========================== Second Layer Part ===========================*/
	//GtkWidget * tabs, *label_voice, *label_song, *tempLabel, *vbox;
	//tabs = gtk_notebook_new();
	//label_voice = gtk_label_new("Voice");
	//label_song = gtk_label_new("Instrument");

	//songProgressBox = gtk_hbox_new(false, 0);
	//voiceProgressBox = gtk_hbox_new(false, 0);

	//gtk_notebook_append_page(GTK_NOTEBOOK(tabs), songProgressBox, label_song);
	//gtk_notebook_append_page(GTK_NOTEBOOK(tabs), voiceProgressBox, label_voice);
	//g_signal_connect(G_OBJECT(tabs), "switch-page", G_CALLBACK(tabSwitchPage), NULL);
	//
	///*Instrument Tab*/
	//vbox = gtk_vbox_new(false, 0);
	///*label for loudness meter*/
	//tempLabel = gtk_label_new("Loudness Meter (db):");
	//gtk_box_pack_start(GTK_BOX(vbox), tempLabel, false, false, 5);
	///*label for frequency*/
	//tempLabel = gtk_label_new("Frequency (Hz):");
	//gtk_box_pack_start(GTK_BOX(vbox), tempLabel, false, false, 5);
	//gtk_box_pack_start(GTK_BOX(songProgressBox), vbox, false, false, 5);

	///*getting the second vbox*/
	//vbox = gtk_vbox_new(false, 0);
	///*intensity bar*/
	intensityProgBarInstru = gtk_progress_bar_new();
	gtk_widget_set_size_request(GTK_WIDGET(intensityProgBarInstru), 100, 20);
	//gtk_box_pack_start(GTK_BOX(overAllWindowBox), intensityProgBarInstru, false, false, 5);
	//
	///*frequency label*/
	//freqLabelInstru = gtk_label_new("");
	//gtk_box_pack_start(GTK_BOX(vbox), freqLabelInstru, false, false, 5);
	//gtk_box_pack_start(GTK_BOX(songProgressBox), vbox, false, false, 5);

	///*Voice Tab*/
	//vbox = gtk_vbox_new(false, 0);
	///*label for loudness meter*/
	//tempLabel = gtk_label_new("Loudness Meter (db):");
	//gtk_box_pack_start(GTK_BOX(vbox), tempLabel, false, false, 5);
	///*label for frequency*/
	//tempLabel = gtk_label_new("Frequency (Hz):");
	//gtk_box_pack_start(GTK_BOX(vbox), tempLabel, false, false, 5);
	//gtk_box_pack_start(GTK_BOX(voiceProgressBox), vbox, false, false, 5);

	///*getting the second vbox*/
	//vbox = gtk_vbox_new(false, 0);
	///*intensity bar*/
	//intensityProgBarVoice = gtk_progress_bar_new();
	////gtk_widget_set_size_request(GTK_WIDGET(intensityProgressBar), 100, 20);
	//gtk_box_pack_start(GTK_BOX(vbox), intensityProgBarVoice, false, false, 5);

	///*frequency label*/
	//freqLabelVoice = gtk_label_new("");
	//gtk_box_pack_start(GTK_BOX(vbox), freqLabelVoice, false, false, 5);
	//gtk_box_pack_start(GTK_BOX(voiceProgressBox), vbox, false, false, 5);


	//gtk_widget_set_size_request(GTK_WIDGET(tabs), 300, 100);
	//gtk_window_set_resizable(GTK_WINDOW(tabs), FALSE);
	////gtk_box_pack_start(GTK_BOX(tabs), songProgressBox, false, false, 5);
	//gtk_box_pack_start(GTK_BOX(tabBox), tabs, false, false, 5);

	/*========================================== First Layer Part ==========================================*/
	/*frequency combo boxes*/

	/*frequency combo boxes*/
	GtkListStore *freqList = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_STRING);
	gtk_list_store_insert_with_values(freqList, NULL, -1, 0, "red", 1, "", -1);
	gtk_list_store_insert_with_values(freqList, NULL, -1, 0, "green", 1, " ", -1);
	gtk_list_store_insert_with_values(freqList, NULL, -1, 0, "blue", 1, " ", -1);

	freqHighColours = gtk_combo_box_new_with_model(GTK_TREE_MODEL(freqList));
	freqLowColours = gtk_combo_box_new_with_model(GTK_TREE_MODEL(freqList));
	tempoColours = gtk_combo_box_new_with_model(GTK_TREE_MODEL(freqList));

	column = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(freqHighColours), column, false);
	gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(freqHighColours), column, "cell-background", 0, "text", 1, NULL);
	gtk_widget_set_size_request(freqHighColours, 100, 20);

	gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(freqLowColours), column, false);
	gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(freqLowColours), column, "cell-background", 0, "text", 1, NULL);
	gtk_widget_set_size_request(freqLowColours, 100, 20);

	gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(tempoColours), column, false);
	gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(tempoColours), column, "cell-background", 0, "text", 1, NULL);
	gtk_widget_set_size_request(tempoColours, 100, 20);


	gtk_combo_box_set_active(GTK_COMBO_BOX(freqHighColours), 2);
	gtk_combo_box_set_active(GTK_COMBO_BOX(freqLowColours), 0);
	gtk_combo_box_set_active(GTK_COMBO_BOX(tempoColours), 1);

	/*Frequency Control Box*/

	/*the frame*/
	freqControlFrame = gtk_frame_new("Frequency");
	gtk_widget_set_size_request(freqControlFrame, 150, 100);
	gtk_box_pack_start(GTK_BOX(freqControlBox), freqControlFrame, false, false, 5);
	GtkWidget *vbox1 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	gtk_container_add(GTK_CONTAINER(freqControlFrame), vbox1);

	GtkWidget *hbox1 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);

	/*label for low*/
	tempLabel = gtk_label_new("Low: ");
	gtk_box_pack_start(GTK_BOX(hbox1), tempLabel, false, false, 5);

	/*drop down for low*/
	gtk_box_pack_start(GTK_BOX(hbox1), freqLowColours, false, false, 5);
	gtk_box_pack_start(GTK_BOX(vbox1), hbox1, false, false, 5);

	GtkWidget *hbox2 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);

	/*label for high*/
	tempLabel = gtk_label_new("High:");
	gtk_box_pack_start(GTK_BOX(hbox2), tempLabel, false, false, 5);

	/*drop down for high*/
	gtk_box_pack_start(GTK_BOX(hbox2), freqHighColours, false, false, 5);
	gtk_box_pack_start(GTK_BOX(vbox1), hbox2, false, false, 5);


	/*Tempo Control Box*/
	tempoControlFrame = gtk_frame_new("Tempo");
	gtk_box_pack_start(GTK_BOX(tempoControlBox), tempoControlFrame, false, false, 5);
	GtkWidget *hbox3 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	gtk_container_add(GTK_CONTAINER(tempoControlFrame), hbox3);

	/*Label for high*/
	tempLabel = gtk_label_new("High: ");
	gtk_box_pack_start(GTK_BOX(hbox3), tempLabel, false, false, 5);

	/*drop down for high*/
	gtk_box_pack_start(GTK_BOX(hbox3), tempoColours, false, false, 5);

	/*play button*/
	playButton = gtk_button_new_with_label("Play");
	gtk_box_pack_start(GTK_BOX(songControlBox), playButton, true, true, 5);
	g_signal_connect(GTK_BUTTON(playButton), "clicked", G_CALLBACK(startJamming), window);
	gtk_widget_show(playButton);

	/*pause button*/
	playButton = gtk_button_new_with_label("Stop");
	gtk_box_pack_start(GTK_BOX(songControlBox), playButton, true, true, 5);
	g_signal_connect(GTK_BUTTON(playButton), "clicked", G_CALLBACK(CloseAllThreads), NULL);
	gtk_widget_show(playButton);

	/*test button*/
	testButton = gtk_button_new_with_label("Test");
	//gtk_box_pack_start(GTK_BOX(songControlBox), testButton, false, false, 5);
	g_signal_connect(GTK_BUTTON(testButton), "clicked", G_CALLBACK(changeProgressBar), NULL);
	gtk_widget_show(testButton);

	gtk_container_add(GTK_CONTAINER(radioButtonFrame), numChannelBox);

	freeplay = gtk_radio_button_new_with_label(NULL, "Freeplay");
	gtk_box_pack_start(GTK_BOX(numChannelBox), freeplay, false, false, 5);

	concert = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(freeplay), "Concert");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(concert), true);
	gtk_box_pack_start(GTK_BOX(numChannelBox), concert, false, false, 5);

	/*status label*/
	statusLabel = gtk_label_new("Standby");
	gtk_box_pack_start(GTK_BOX(songStatusBox), statusLabel, true, true, 5);

	/*display lyrics*/
	outputLyrics = gtk_button_new_with_label("Display Lyrics");
	g_signal_connect(GTK_BUTTON(outputLyrics), "clicked", G_CALLBACK(displayLyricsNonmodal), window);
	gtk_box_pack_start(GTK_BOX(lyricsBox), outputLyrics, true, true, 5);

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
		gtk_label_set_text(GTK_LABEL(statusLabel), "Standby");

		stopUpdating();
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