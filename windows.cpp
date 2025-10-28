#include <vector>

#ifdef _WIN32
	#include <curses.h>
#else
	#include <ncurses.h>
#endif

#include "windows.hpp"
#include "gpedit.hpp"
#include "gp_file.hpp"

WINDOW* songInfoWindow;
WINDOW* tabDisplayWindow;
WINDOW* beatInfoWindow;

void displaySongInfo() {
	// create window with height, width, yTop, xLeft
	songInfoWindow = newwin(7, getmaxx(stdscr), 0, 0);
	box(songInfoWindow, 0, 0);
	
	wattron(songInfoWindow, A_REVERSE);
	wprintw(songInfoWindow, "File open: ");
	wattron(songInfoWindow, A_ITALIC);
	wprintw(songInfoWindow, "%s", songFilePath.substr(songFilePath.find_last_of("/\\") + 1).c_str());
	wattroff(songInfoWindow, A_ITALIC);
	wattroff(songInfoWindow, A_REVERSE);
	
	wattron(songInfoWindow, A_BOLD);
	wmove(songInfoWindow, 2, 2);
	wprintw(songInfoWindow, "Title:");
	wmove(songInfoWindow, 3, 2);
	wprintw(songInfoWindow, "Artist:");
	wmove(songInfoWindow, 4, 2);
	wprintw(songInfoWindow, "Comments:");
	wattroff(songInfoWindow, A_BOLD);
	
	wmove(songInfoWindow, 2, 12);
	wprintw(songInfoWindow, "%s", song.metadata.title.c_str());
	wmove(songInfoWindow, 3, 12);
	wprintw(songInfoWindow, "%s", song.metadata.artist.c_str());
	wmove(songInfoWindow, 4, 12);
	wprintw(songInfoWindow, "%s", song.metadata.instructions.c_str());
	
	refresh();
	wrefresh(songInfoWindow);
}

void selectTrack() {
	std::string trackList[song.trackCount];
	for (int i = 0; i < song.trackCount; i++) {
		trackList[i] = song.trackHeaders[i].name;
	}
	
	WINDOW* selectTrack = newwin(song.trackCount + 4, 15, getmaxy(songInfoWindow), 0);
	box(selectTrack, 0, 0);
	
	wattron(selectTrack, A_REVERSE);
	wprintw(selectTrack, "Select Track");
	wattroff(selectTrack, A_REVERSE);
	
	refresh();
	wrefresh(selectTrack);
	
	// allow reading non-character keypresses
	keypad(selectTrack, true);
	
	while (true) {
		for (int i = 0; i < song.trackCount; i++) {
			if (i == trackIndex) {
				wattron(selectTrack, A_REVERSE);
			}
			mvwprintw(selectTrack, i+2, 2, "%s", trackList[i].c_str());
			wattroff(selectTrack, A_REVERSE);
		}
		
		keyboardInput = wgetch(selectTrack);
		
		switch (keyboardInput) {
			case KEY_UP:
				if (trackIndex > 0) {
					trackIndex--;
				}
				break;
			case KEY_DOWN:
				if (trackIndex < song.trackCount - 1) {
					trackIndex++;
				}
				break;
		}
		if (keyboardInput == 10 || keyboardInput == 27) {
			break;
		}
	}
	
	wclear(selectTrack);
	wrefresh(selectTrack);
	delwin(selectTrack);
	refresh();
}

void initTabDisplay() {
	int tabWindowHeight = 11;
	int yTop = getmaxy(songInfoWindow);
	
	// create tab window
	tabDisplayWindow = newwin(tabWindowHeight, getmaxx(stdscr), yTop, 0);
	box(tabDisplayWindow, 0, 0);
	wattron(tabDisplayWindow, A_REVERSE);
	wprintw(tabDisplayWindow, "Track: %s", song.trackHeaders[trackIndex].name.c_str());
	wattroff(tabDisplayWindow, A_REVERSE);
	
	// create beat info window
	beatInfoWindow = newwin(11, getmaxx(stdscr)/2, yTop+tabWindowHeight, 0);
	box(beatInfoWindow, 0, 0);
	wattron(beatInfoWindow, A_REVERSE);
	wprintw(beatInfoWindow, "Beat Info");
	wattroff(beatInfoWindow, A_REVERSE);
	
	refresh();
	wrefresh(tabDisplayWindow);
	wrefresh(beatInfoWindow);
}

void printBeatInfo(int measureIndex, int beatIndex, bool isRest) {
	mvwprintw(beatInfoWindow, 1, 1, "Measure: %d  ", measureIndex+1);
	mvwprintw(beatInfoWindow, 2, 1, "Beat: %d  ", beatIndex+1);
	mvwprintw(beatInfoWindow, 3, 1, "Rest: %s   ", isRest ? "Yes" : "No");
		
	wrefresh(beatInfoWindow);
}