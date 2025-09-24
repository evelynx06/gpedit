#include <iostream>
#include <fstream>
#include <ncurses.h>

#include "GPFile.hpp"

WINDOW* songInfoWindow(std::string songFileName, GPFile song);
int selectTrack(int yTop, int xLeft, GPFile song);
int editTab(int yTop, int xLeft, int trackIndex, GPFile& song);


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
	WINDOW* tabDisplay = newwin(15, getmaxx(stdscr), yTop, xLeft);
	box(tabDisplay, 0, 0);
	
	wattron(tabDisplay, A_REVERSE);
	wprintw(tabDisplay, "Track: %s", song.trackHeaders[trackIndex].name.c_str());
	wattroff(tabDisplay, A_REVERSE);
	
	refresh();
	wrefresh(tabDisplay);
	return 0;
}