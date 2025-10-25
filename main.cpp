#include <iostream>
#include <fstream>
#include <vector>

#ifdef _WIN32
	#include <curses.h>
#else
	#include <ncurses.h>
#endif

#include "GPFile.hpp"


struct DisplayedBeat {
	int beatOffset;
	int beatWidth;
	int measureIndex;
	int beatIndex;
};


WINDOW* songInfoWindow(std::string songFileName, GPFile song);
int selectTrack(int yTop, int xLeft, GPFile song);
int editTab(int yTop, int xLeft, int trackIndex, GPFile& song);
std::string getStringName(int tuningValue);
std::vector<DisplayedBeat> printBeats(WINDOW* tabDisplay, GPFile song, int trackIndex, int startingMeasure, int startingBeat);


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
	while (editTab(getmaxy(songInfo), 0, trackIndex, song) == 27) {
		trackIndex = selectTrack(getmaxy(songInfo), 0, song);
		if (trackIndex < 0) {
			break;
		}
	}
	
	wclear(songInfo);
	wrefresh(songInfo);
	delwin(songInfo);
	refresh();
	
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
	wprintw(songInfo, "%s", songFileName.c_str());
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
	wprintw(songInfo, "%s", song.metadata.title.c_str());
	wmove(songInfo, 3, 12);
	wprintw(songInfo, "%s", song.metadata.artist.c_str());
	wmove(songInfo, 4, 12);
	wprintw(songInfo, "%s", song.metadata.instructions.c_str());
	
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
			mvwprintw(selectTrack, i+2, 2, "%s", trackList[i].c_str());
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
		if (input == 10 || input == 27) {
			break;
		}
	}
	
	wclear(selectTrack);
	wrefresh(selectTrack);
	delwin(selectTrack);
	refresh();
	
	return input == 27 ? -1 : highlight;
}

int editTab(int yTop, int xLeft, int trackIndex, GPFile& song) {
	TrackHeader track = song.trackHeaders[trackIndex];
	
	int tabWindowHeight = 11;
	
	// create tab window
	WINDOW* tabDisplay = newwin(tabWindowHeight, getmaxx(stdscr), yTop, xLeft);
	box(tabDisplay, 0, 0);
	wattron(tabDisplay, A_REVERSE);
	wprintw(tabDisplay, "Track: %s", track.name.c_str());
	wattroff(tabDisplay, A_REVERSE);
	
	// create beat info window
	WINDOW* beatInfo = newwin(11, getmaxx(stdscr)/2, yTop+tabWindowHeight, xLeft);
	box(beatInfo, 0, 0);
	wattron(beatInfo, A_REVERSE);
	wprintw(beatInfo, "Beat Info");
	wattroff(beatInfo, A_REVERSE);
	
	refresh();
	wrefresh(beatInfo);
	wrefresh(tabDisplay);
	
	int startingMeasure = 0;
	int startingBeat = 0;
	
	std::vector<DisplayedBeat> displayedBeats;
	displayedBeats = printBeats(tabDisplay, song, trackIndex, startingMeasure, startingBeat);
	
	// allow reading non-character keypresses
	keypad(tabDisplay, true);
	
	int input;
	int selectionIndex = 0;
	int stringIndex = 0;
	
	
	while (true) {
		DisplayedBeat selectedBeat = displayedBeats[selectionIndex];
		
		char selection[selectedBeat.beatWidth-1];
		mvwinnstr(tabDisplay, stringIndex+3, selectedBeat.beatOffset, selection, selectedBeat.beatWidth-1);
		
		wattron(tabDisplay, A_REVERSE);
		mvwprintw(tabDisplay, stringIndex+3, selectedBeat.beatOffset, "%s", selection);
		wattroff(tabDisplay, A_REVERSE);
		
		Beat beat = song.measures[selectedBeat.measureIndex][trackIndex].beats[selectedBeat.beatIndex];
		
		mvwprintw(beatInfo, 1, 1, "Measure: %d  ", selectedBeat.measureIndex+1);
		mvwprintw(beatInfo, 2, 1, "Beat: %d  ", selectedBeat.beatIndex+1);
		mvwprintw(beatInfo, 3, 1, "Rest: %s   ",  beat.isRest ? "Yes" : "No");
		
		wrefresh(tabDisplay);
		wrefresh(beatInfo);
		
		input = wgetch(tabDisplay);
		
		switch (input) {
			case KEY_LEFT:
				if (selectionIndex > 0) {
					mvwprintw(tabDisplay, stringIndex+3, selectedBeat.beatOffset, "%s", selection);
					selectionIndex--;
				}
				else if (selectedBeat.beatIndex > 0) {
					startingBeat--;
					displayedBeats = printBeats(tabDisplay, song, trackIndex, startingMeasure, startingBeat);
				}
				else if (selectedBeat.measureIndex > 0) {
					startingMeasure--;
					startingBeat = song.measures[startingMeasure][trackIndex].beatCount-1;
					displayedBeats = printBeats(tabDisplay, song, trackIndex, startingMeasure, startingBeat);
				}
				break;
			case KEY_RIGHT:
				if (selectionIndex < displayedBeats.size() - 1) {
					mvwprintw(tabDisplay, stringIndex+3, selectedBeat.beatOffset, "%s", selection);
					selectionIndex++;
				}
				else if (startingBeat < song.measures[startingMeasure][trackIndex].beatCount-1) {
					startingBeat++;
					displayedBeats = printBeats(tabDisplay, song, trackIndex, startingMeasure, startingBeat);
					selectionIndex = displayedBeats.size()-1;
				}
				else if (startingMeasure < song.measureCount-2) {
					startingMeasure++;
					startingBeat = 0;
					displayedBeats = printBeats(tabDisplay, song, trackIndex, startingMeasure, startingBeat);
					selectionIndex = displayedBeats.size()-1;
				}
				break;
			case KEY_UP:
				if (stringIndex > 0) {
					mvwprintw(tabDisplay, stringIndex+3, selectedBeat.beatOffset, "%s", selection);
					stringIndex--;
				}
				break;
			case KEY_DOWN:
				if (stringIndex < track.stringCount - 1) {
					mvwprintw(tabDisplay, stringIndex+3, selectedBeat.beatOffset, "%s", selection);
					stringIndex++;
				}
				break;
			case KEY_SLEFT:
				if (startingMeasure > 0) {
					startingMeasure--;
					displayedBeats = printBeats(tabDisplay, song, trackIndex, startingMeasure, 0);
				}
				break;
			case KEY_SRIGHT:
				if (startingMeasure < song.measureCount-2) {
					startingMeasure++;
					displayedBeats = printBeats(tabDisplay, song, trackIndex, startingMeasure, 0);
				}
				break;
			default:
				break;
		}
		if (input == 27) {
			break;
		}
	}
	
	wclear(tabDisplay);
	wclear(beatInfo);
	wrefresh(tabDisplay);
	wrefresh(beatInfo);
	delwin(tabDisplay);
	delwin(beatInfo);
	refresh();
	return input;
}


// the string tuning value is stored as an integer corresponding to its MIDI note value
// the MIDI note value represents the number of semitones above the lowest note, C(-1)
std::string getStringName(int tuningValue) {
	std::string noteNames[] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
	
	int noteIndex = tuningValue % 12;
	int octave = (tuningValue / 12) - 1;
	
	return noteNames[noteIndex] + std::to_string(octave);
}

std::vector<DisplayedBeat> printBeats(WINDOW* tabDisplay, GPFile song, int trackIndex, int startingMeasure = 0, int startingBeat = 0) {
	TrackHeader track = song.trackHeaders[trackIndex];
	
	int leftMargin = 1;
	int rightMargin = 2;
	int topMargin = 3;
	int bottomMargin = 1;
	
	// print string names
	std::string stringBeginning;
	if (startingBeat != 0) {
		stringBeginning = ":-";
	}
	else if (startingMeasure != 0) {
		stringBeginning = ":|-";
	}
	else {
		stringBeginning = "|-";
	}
	for (int stringIndex = 0; stringIndex < track.stringCount; stringIndex++) {
		mvwprintw(tabDisplay, stringIndex+topMargin, leftMargin, "%s", getStringName(track.stringTuning[stringIndex]).c_str());
		mvwprintw(tabDisplay, stringIndex+topMargin, 4, "%s", stringBeginning.c_str());
	}
	leftMargin = getcurx(tabDisplay);
	
	int xMax = getmaxx(tabDisplay) - rightMargin;
	
	// print tab base
	std::string stringBase = std::string(xMax-leftMargin - 1, '-').append(":");
	for (int stringIndex = 0; stringIndex < track.stringCount; stringIndex++) {
		mvwprintw(tabDisplay, topMargin+stringIndex, leftMargin, "%s", stringBase.c_str());
	}
	std::string durationClear = std::string(xMax-leftMargin, ' ');
	mvwprintw(tabDisplay, topMargin-2, leftMargin-1, "%s", durationClear.c_str());
	mvwprintw(tabDisplay, topMargin-1, leftMargin-1, "%s", durationClear.c_str());
	
	std::vector<DisplayedBeat> displayedBeats;
	
	int measureIndex = startingMeasure;
	int beatIndex = startingBeat;
	Measure measure = song.measures[measureIndex][trackIndex];
	
	int beatOffset = leftMargin;	// the cursor position at the start of the current beat (or other printed section, such as bar lines)
	wmove(tabDisplay, 0, beatOffset);
	
	// print beats as long as there is room left
	while (getcurx(tabDisplay)+6 < xMax) {
		Beat beat = measure.beats[beatIndex];
		
		beatOffset = getcurx(tabDisplay);
				
		std::string beatDuration;
		switch (beat.duration) {
			case gp_duration_whole:
				beatDuration = "w";
				break;
			case gp_duration_half:
				beatDuration = "h";
				break;
			case gp_duration_quarter:
				beatDuration = "q";
				break;
			case gp_duration_eighth:
				beatDuration = "e";
				break;
			case gp_duration_sixteenth:
				beatDuration = "s";
				break;
			case gp_duration_thirty_second:
				beatDuration = "t";
				break;
			case gp_duration_sixty_fourth:
				beatDuration = "S";
				break;
		}
		if (beat.beatFlags & gp_beat_is_dotted) {
			beatDuration.append(".");
		}
		if (beat.beatFlags & gp_beat_is_tuplet) {
			mvwprintw(tabDisplay, topMargin-2, beatOffset, "%s", std::to_string(beat.tupletDivision).c_str());
		}
		mvwprintw(tabDisplay, topMargin-1, beatOffset, "%s", beatDuration.c_str());
		
		int maxBeatWidth = 0;	// keeps track of the maximum printed width of the beat
		int beatWidth;	// printed beat width of current string
		
		for (int stringIndex = 0; stringIndex < track.stringCount; stringIndex++) {	// loop through the strings
			// set cursor position to start of beat, on current string
			wmove(tabDisplay, topMargin+stringIndex, beatOffset);
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
					wprintw(tabDisplay, "%s", fret.c_str());
					beatWidth += fret.length();
				}
				
				if (note.noteFlags & gp_note_is_ghost) {
					wprintw(tabDisplay, ")");
					beatWidth++;
				}
				
				if (note.noteFlags & gp_note_is_accent) {
					wprintw(tabDisplay, ">");
					beatWidth++;
				}
				
				if (beat.beatFlags & gp_beat_has_effects) {
					if (beat.effects.beatEffectFlags & gp_beatfx_vibrato) {
						wprintw(tabDisplay, "~");
						beatWidth++;
					}
					
					if (beat.effects.beatEffectFlags & gp_beatfx_natural_harmonic) {
						wprintw(tabDisplay, "+");
						beatWidth++;
					}
					
					if (beat.effects.beatEffectFlags & gp_beatfx_tremolo_or_tap) {
						if (beat.effects.tremoloOrTap == 1) {	// tap
							wprintw(tabDisplay, "t");
							beatWidth++;
						}
					}
				}
				
				if (note.noteFlags & gp_note_has_effects) {
					if (note.noteEffectFlags & gp_notefx_bend) {
						switch (note.noteBend.type) {
							case gp_bendtype_bend:
								wprintw(tabDisplay, "b");
								beatWidth++;
								break;
							case gp_bendtype_bend_release:
								wprintw(tabDisplay, "br");
								beatWidth += 2;
								break;
							case gp_bendtype_bend_release_bend:
								wprintw(tabDisplay, "brb");
								beatWidth += 3;
								break;
							case gp_bendtype_prebend:
								wprintw(tabDisplay, "pb");
								beatWidth += 2;
								break;
							case gp_bendtype_prebend_release:
								wprintw(tabDisplay, "pbr");
								beatWidth += 3;
								break;
							default:
								break;
						}
					}
					
					if (note.noteEffectFlags & gp_notefx_hammer_pull) {
						Note followingNote = measure.beats[beatIndex + 1].beatNotes.strings[stringIndex];
						
						if (followingNote.fretNumber < note.fretNumber) {
							wprintw(tabDisplay, "p");
						}
						else {
							wprintw(tabDisplay, "h");
						}
						// beatWidth is not incremented, cause there shouldn't be any space before the next note
					}
					
					if (note.noteEffectFlags & gp_notefx_slide) {
						Note followingNote = measure.beats[beatIndex + 1].beatNotes.strings[stringIndex];
						
						if (followingNote.fretNumber < note.fretNumber) {
							wprintw(tabDisplay, "\\");
						}
						else {
							wprintw(tabDisplay, "/");
						}
						// beatWidth is not incremented, cause there shouldn't be any space before the next note
					}
					
					if (note.noteEffectFlags & gp_notefx_let_ring) {
						// TODO: figure something out
					}
					
					if (note.noteEffectFlags & gp_notefx_grace_note) {
						// TODO: figure something out
						// being a grace note is not a property of a note,
						// but rather a grace note is attatched to the note it preceeds,
						// meaning it has to be printed before it
					}
				}
			}
			else {	// string is not played
				beatWidth++;
			}
			
			maxBeatWidth = beatWidth > maxBeatWidth ? beatWidth : maxBeatWidth;
		}
		
				
		displayedBeats.push_back(DisplayedBeat{ beatOffset, maxBeatWidth, measureIndex, beatIndex });
		
		wmove(tabDisplay, 0, beatOffset+maxBeatWidth);
		
		if (beatIndex+1 >= measure.beatCount) {	// check if end of measure reached	
			if (measureIndex+1 >= song.measureCount) {	// check if end of song reached
				// clear the rest of the tab area
				beatOffset = getcurx(tabDisplay);
				std::string clearString = "|";
				clearString.append(std::string(xMax-beatOffset, ' '));
				for (int stringIndex = 0; stringIndex < track.stringCount; stringIndex++) {
					mvwprintw(tabDisplay, topMargin+stringIndex, beatOffset, "%s", clearString.c_str());
				}
				wrefresh(tabDisplay);
				return displayedBeats;
			}
			
			beatIndex = 0;
			measureIndex++;
			
			// measure index has changed, so get the new measure object
			measure = song.measures[measureIndex][trackIndex];
			
			// print bar line
			beatOffset = getcurx(tabDisplay);
			for (int stringIndex = 0; stringIndex < track.stringCount; stringIndex++) {
				mvwprintw(tabDisplay, topMargin+stringIndex, beatOffset, "|-");
			}
		}
		else {
			beatIndex++;
		}
	}
	
	wrefresh(tabDisplay);
	return displayedBeats;
}