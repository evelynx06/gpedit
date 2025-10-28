#ifndef EDITING_H
#define EDITING_H

#include <string>
#include <vector>

#ifdef _WIN32
	#include <curses.h>
#else
	#include <ncurses.h>
#endif

struct DisplayedBeat {
	int beatOffset;
	int beatWidth;
	int measureIndex;
	int beatIndex;
};

std::string getStringName(int tuningValue);
std::vector<DisplayedBeat> printBeats(int startingMeasure, int startingBeat);
void editTab();

#endif // !EDITING_H