#include <iostream>

#ifdef _WIN32
	#include <curses.h>
#else
	#include <ncurses.h>
#endif

#include "gpedit.hpp"
#include "windows.hpp"
#include "editing.hpp"


int main(int argc, char const *argv[]) {
	if (argc != 2) {
		std::cerr << "Invalid arguments.\n\nUsage: gpedit FILE\n";
		return 1;
	}
	
	if(openFile(argv[1]) != 0) {
		return 1;
	}
	
	/* NCURSES START */
	initscr();
	
	// allow Ctrl-C to exit
	cbreak();
	// don't print keypresses
	noecho();
	// hide cursor
	curs_set(0);
	
	
	displaySongInfo();
	
	while (true) {
		selectTrack();
		if (keyboardInput == 27) {
			break;
		}
		editTab();
	}
	
	
	wclear(songInfoWindow);
	wrefresh(songInfoWindow);
	delwin(songInfoWindow);
	refresh();
	
	/* NCURSES END */
	endwin();
	
	return 0;
}