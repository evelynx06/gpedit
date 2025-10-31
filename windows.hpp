#ifndef WINDOWS_H
#define WINDOWS_H

#include "editing.hpp"
#ifdef _WIN32
	#include <curses.h>
#else
	#include <ncurses.h>
#endif

extern WINDOW* songInfoWindow;
extern WINDOW* tabDisplayWindow;
extern WINDOW* beatInfoWindow;

void displaySongInfo();
void selectTrack();
void initTabDisplay();
void printBeatInfo(DisplayedBeat selectedBeat, int stringIndex);

#endif // !WINDOWS_H