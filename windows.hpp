#ifndef WINDOWS_H
#define WINDOWS_H

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
void printBeatInfo(int measureIndex, int beatIndex, bool isRest);

#endif // !WINDOWS_H