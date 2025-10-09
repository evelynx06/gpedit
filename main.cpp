#include <iostream>
#include <fstream>
#include <ncurses.h>

#include "GPFile.hpp"

WINDOW* songInfoWindow(std::string songFileName, GPFile song);
int selectTrack(int yTop, int xLeft, GPFile song);
int editTab(int yTop, int xLeft, int trackIndex, GPFile& song);
std::string getStringName(int tuningValue);
int printBeats(WINDOW* tabDisplay, GPFile song, int trackIndex, int startingMeasure, int startingBeat);


int main(int argc, char const *argv[]) {
	if (argc != 2) {
		std::cerr << "Invalid arguments.\n\nUsage: gpedit FILE\n";
		return 1;
	}
	
	// open file
	std::string filePath = argv[1];
	std::ifstream fileStream(filePath, std::ios::in|std::ios::binary);
	if (!fileStream) {
		std::cerr << "Error opening file.\n";
		return 1;
	}
	
	// read file
	GPFile song(fileStream);
	
	fileStream.close();
	
	
	/* NCURSES START */
	initscr();
	
	// allow Ctrl-C to exit
	cbreak();
	// don't print keypresses
	noecho();
	// hide cursor
	curs_set(0);
	
	
	// display song metadata
	WINDOW* songInfo = songInfoWindow(filePath.substr(filePath.find_last_of("/\\") + 1), song);
	refresh();
	wrefresh(songInfo);
	
	// let the user select which track to edit
	int trackIndex = selectTrack(getmaxy(songInfo), 0, song);
	
	// start editing
	editTab(getmaxy(songInfo), 0, trackIndex, song);

	
	// wait for keypress
	getch();	
	
	/* NCURSES END */
	endwin();
	
	return 0;
}

WINDOW* songInfoWindow(std::string songFileName, GPFile song) {
	// create window with height, width, yTop, xLeft
	WINDOW* songInfo = newwin(7, getmaxx(stdscr), 0, 0);
	box(songInfo, 0, 0);
	// wborder(songInfo, 0x20, ACS_VLINE, 0x20, ACS_HLINE, 0x20, ACS_VLINE, ACS_HLINE, ACS_LRCORNER);
	
	wattron(songInfo, A_REVERSE);
	wprintw(songInfo, "File open: ");
	wattron(songInfo, A_ITALIC);
	wprintw(songInfo, songFileName.c_str());
	wattroff(songInfo, A_ITALIC);
	wattroff(songInfo, A_REVERSE);
	
	wattron(songInfo, A_BOLD);
	wmove(songInfo, 2, 2);
	wprintw(songInfo, "Title:");
	wmove(songInfo, 3, 2);
	wprintw(songInfo, "Artist:");
	wmove(songInfo, 4, 2);
	wprintw(songInfo, "Comments:");
	wattroff(songInfo, A_BOLD);
	
	wmove(songInfo, 2, 12);
	wprintw(songInfo, song.metadata.title.c_str());
	wmove(songInfo, 3, 12);
	wprintw(songInfo, song.metadata.artist.c_str());
	wmove(songInfo, 4, 12);
	wprintw(songInfo, song.metadata.instructions.c_str());
	
	return songInfo;
}

int selectTrack(int yTop, int xLeft, GPFile song) {
	std::string trackList[song.trackCount];
	for (int i = 0; i < song.trackCount; i++) {
		trackList[i] = song.trackHeaders[i].name;
	}
	
	WINDOW* selectTrack = newwin(song.trackCount + 4, 15, yTop, xLeft);
	box(selectTrack, 0, 0);
	
	wattron(selectTrack, A_REVERSE);
	wprintw(selectTrack, "Select Track");
	wattroff(selectTrack, A_REVERSE);
	
	refresh();
	wrefresh(selectTrack);
	
	// allow reading non-character keypresses
	keypad(selectTrack, true);
	
	int input;
	int highlight = 0;
	
	while (true) {
		for (int i = 0; i < song.trackCount; i++) {
			if (i == highlight) {
				wattron(selectTrack, A_REVERSE);
			}
			mvwprintw(selectTrack, i+2, 2, trackList[i].c_str());
			wattroff(selectTrack, A_REVERSE);
		}
		
		input = wgetch(selectTrack);
		
		switch (input) {
			case KEY_UP:
				if (highlight > 0) {
					highlight--;
				}
				break;
			case KEY_DOWN:
				if (highlight < song.trackCount - 1) {
					highlight++;
				}
				break;
			default:
				break;
		}
		if (input == 10) {
			break;
		}		
	}
	
	wclear(selectTrack);
	wrefresh(selectTrack);
	delwin(selectTrack);
	refresh();
	
	return highlight;
}

int editTab(int yTop, int xLeft, int trackIndex, GPFile& song) {
	TrackHeader track = song.trackHeaders[trackIndex];
	
	
	// create window with border and title
	WINDOW* tabDisplay = newwin(15, getmaxx(stdscr), yTop, xLeft);
	box(tabDisplay, 0, 0);
	wattron(tabDisplay, A_REVERSE);
	wprintw(tabDisplay, "Track: %s", track.name.c_str());
	wattroff(tabDisplay, A_REVERSE);
	
	refresh();
	wrefresh(tabDisplay);
	
	printBeats(tabDisplay, song, trackIndex, 0, 0);
	
	// mvwprintw(tabDisplay, 10, 1, "beatIndex: %d  ", beatIndex);
	// mvwprintw(tabDisplay, 11, 1, "printOffset: %d  ", beatStart);
	// mvwprintw(tabDisplay, 12, 1, "measureIndex: %d  ", measureIndex);
	
	// wrefresh(tabDisplay);
	return 0;
}


// the string tuning value is stored as an integer corresponding to its MIDI note value
// the MIDI note value represents the number of semitones above the lowest note, C(-1)
std::string getStringName(int tuningValue) {
	std::string noteNames[] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
	
	int noteIndex = tuningValue % 12;
	int octave = (tuningValue / 12) - 1;
	
	return noteNames[noteIndex] + std::to_string(octave);
}

int printBeats(WINDOW* tabDisplay, GPFile song, int trackIndex, int startingMeasure = 0, int startingBeat = 0) {
	TrackHeader track = song.trackHeaders[trackIndex];
	
	int leftMargin = 1;
	int rightMargin = 2;
	int topMargin = 2;
	int bottomMargin = 1;
	
	// print string names
	for (int i = 0; i < track.stringCount; i++) {
		mvwprintw(tabDisplay, i+topMargin, leftMargin, getStringName(track.stringTuning[i]).c_str());
		mvwprintw(tabDisplay, i+topMargin, 4, "|-");
	}
	leftMargin += 5;
	
	int xMax = getmaxx(tabDisplay) - rightMargin;
	
	// print tab base
	std::string stringBase = std::string(xMax-leftMargin - 1, '-').append(":");
	for (int stringIndex = 0; stringIndex < track.stringCount; stringIndex++) {
		mvwprintw(tabDisplay, topMargin+stringIndex, leftMargin, stringBase.c_str());
	}
	
	
	int measureIndex = startingMeasure;
	int beatIndex = startingBeat;
	Measure measure = song.measures[measureIndex][trackIndex];
	
	int beatStart = leftMargin;	// the cursor position at the start of the current beat (or other printed section, such as bar lines)
	wmove(tabDisplay, 0, beatStart);
	
	// print beats as long as there is room left
	while (getcurx(tabDisplay)+6 < xMax) {
		if (beatIndex+1 >= measure.beatCount) {	// check if end of measure reached	
			if (measureIndex+1 >= song.measureCount) {	// check if end of song reached
				// clear the rest of the tab area
				beatStart = getcurx(tabDisplay);
				std::string clearString = "|";
				clearString.append(std::string(xMax-beatStart, ' '));
				for (int stringIndex = 0; stringIndex < track.stringCount; stringIndex++) {
					mvwprintw(tabDisplay, topMargin+stringIndex, beatStart, clearString.c_str());
				}
				wrefresh(tabDisplay);
				return 0;
			}
			
			beatIndex = 0;
			measureIndex++;
			
			// measure index has changed, so get the new measure object
			measure = song.measures[measureIndex][trackIndex];
			
			// print bar line
			beatStart = getcurx(tabDisplay);
			for (int stringIndex = 0; stringIndex < track.stringCount; stringIndex++) {
				mvwprintw(tabDisplay, topMargin+stringIndex, beatStart, "|-");
			}
		}
		else {
			beatIndex++;
		}
		// beat index changed, so get the new beat object
		Beat beat = measure.beats[beatIndex];
		
		beatStart = getcurx(tabDisplay);
		
		int maxBeatWidth = 0;	// keeps track of the maximum printed width of the beat
		int beatWidth;	// printed beat width of current string
		// int stringsBeatWidth[track.stringCount];	// keeps track of the printed width of the beat for each string
		
		for (int stringIndex = 0; stringIndex < track.stringCount; stringIndex++) {	// loop through the strings
			// set cursor position to start of beat, on current string
			wmove(tabDisplay, topMargin+stringIndex, beatStart);
			beatWidth = 1;
			
			if (beat.beatNotes.stringsPlayed & (0x40 >> stringIndex)) {	// check if string is played
				Note note = beat.beatNotes.strings[stringIndex];	
				
				if (note.noteFlags & gp_note_is_ghost) {
					wprintw(tabDisplay, "(");
					beatWidth++;
				}
				
				if (note.noteType == gp_notetype_dead) {
					wprintw(tabDisplay, "x");
					beatWidth++;
				}
				else if (note.noteType == gp_notetype_tied) {
					wprintw(tabDisplay, "*");
					beatWidth++;
				}
				else {	// note.noteType = gp_notetype_normal
					std::string fret = std::to_string(note.fretNumber);
					wprintw(tabDisplay, fret.c_str());
					beatWidth += fret.length();
				}
				
				if (note.noteFlags & gp_note_is_ghost) {
					wprintw(tabDisplay, ")");
					beatWidth++;
				}
			}
			else {	// string is not played
				beatWidth++;
			}
			
			maxBeatWidth = beatWidth > maxBeatWidth ? beatWidth : maxBeatWidth;
		}
		
		wmove(tabDisplay, 0, beatStart+maxBeatWidth);
	}
	
	wrefresh(tabDisplay);
	return 0;
}