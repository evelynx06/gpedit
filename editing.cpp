#ifdef _WIN32
	#include <curses.h>
#else
	#include <ncurses.h>
#endif

#include "editing.hpp"
#include "gpedit.hpp"
#include "gp_file.hpp"
#include "windows.hpp"

// the string tuning value is stored as an integer corresponding to its MIDI note value
// the MIDI note value represents the number of semitones above the lowest note, C(-1)
std::string getStringName(int tuningValue) {
	std::string noteNames[] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
	
	int noteIndex = tuningValue % 12;
	int octave = (tuningValue / 12) - 1;
	
	return noteNames[noteIndex] + std::to_string(octave);
}

std::vector<DisplayedBeat> printBeats(int startingMeasure = 0, int startingBeat = 0) {
	TrackHeader track = song.trackHeaders[trackIndex];
	
	int leftMargin = 1;
	int rightMargin = 2;
	int topMargin = 3;
	// int bottomMargin = 1;
	
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
		mvwprintw(tabDisplayWindow, stringIndex+topMargin, leftMargin, "%s", getStringName(track.stringTuning[stringIndex]).c_str());
		mvwprintw(tabDisplayWindow, stringIndex+topMargin, 4, "%s", stringBeginning.c_str());
	}
	leftMargin = getcurx(tabDisplayWindow);
	
	int xMax = getmaxx(tabDisplayWindow) - rightMargin;
	
	// print tab base
	std::string stringBase = std::string(xMax-leftMargin - 1, '-').append(":");
	for (int stringIndex = 0; stringIndex < track.stringCount; stringIndex++) {
		mvwprintw(tabDisplayWindow, topMargin+stringIndex, leftMargin, "%s", stringBase.c_str());
	}
	std::string durationClear = std::string(xMax-leftMargin, ' ');
	mvwprintw(tabDisplayWindow, topMargin-2, leftMargin-1, "%s", durationClear.c_str());
	mvwprintw(tabDisplayWindow, topMargin-1, leftMargin-1, "%s", durationClear.c_str());
	
	std::vector<DisplayedBeat> displayedBeats;
	
	int measureIndex = startingMeasure;
	int beatIndex = startingBeat;
	Measure measure = song.measures[measureIndex][trackIndex];
	
	int beatOffset = leftMargin;	// the cursor position at the start of the current beat (or other printed section, such as bar lines)
	wmove(tabDisplayWindow, 0, beatOffset);
	
	// print beats as long as there is room left
	while (getcurx(tabDisplayWindow)+6 < xMax) {
		Beat beat = measure.beats[beatIndex];
		
		beatOffset = getcurx(tabDisplayWindow);
				
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
			mvwprintw(tabDisplayWindow, topMargin-2, beatOffset, "%s", std::to_string(beat.tupletDivision).c_str());
		}
		mvwprintw(tabDisplayWindow, topMargin-1, beatOffset, "%s", beatDuration.c_str());
		
		int maxBeatWidth = 0;	// keeps track of the maximum printed width of the beat
		int beatWidth;	// printed beat width of current string
		
		for (int stringIndex = 0; stringIndex < track.stringCount; stringIndex++) {	// loop through the strings
			// set cursor position to start of beat, on current string
			wmove(tabDisplayWindow, topMargin+stringIndex, beatOffset);
			beatWidth = 1;
			
			if (beat.beatNotes.stringsPlayed & (0x40 >> stringIndex)) {	// check if string is played
				Note note = beat.beatNotes.strings[stringIndex];	
				
				if (note.noteFlags & gp_note_is_ghost) {
					wprintw(tabDisplayWindow, "(");
					beatWidth++;
				}
				
				if (note.noteType == gp_notetype_dead) {
					wprintw(tabDisplayWindow, "x");
					beatWidth++;
				}
				else if (note.noteType == gp_notetype_tied) {
					wprintw(tabDisplayWindow, "*");
					beatWidth++;
				}
				else {	// note.noteType = gp_notetype_normal
					std::string fret = std::to_string(note.fretNumber);
					wprintw(tabDisplayWindow, "%s", fret.c_str());
					beatWidth += fret.length();
				}
				
				if (note.noteFlags & gp_note_is_ghost) {
					wprintw(tabDisplayWindow, ")");
					beatWidth++;
				}
				
				if (note.noteFlags & gp_note_is_accent) {
					wprintw(tabDisplayWindow, ">");
					beatWidth++;
				}
				
				if (note.noteFlags & gp_note_is_heavy_accent) {
					wprintw(tabDisplayWindow, "t^");
					beatWidth++;
				}
				
				if (beat.beatFlags & gp_beat_has_effects) {
					if (beat.effects.beatEffectFlags & gp_beatfx_vibrato) {
						wprintw(tabDisplayWindow, "~");
						beatWidth++;
					}
					
					if (beat.effects.beatEffectFlags & gp_beatfx_natural_harmonic) {
						wprintw(tabDisplayWindow, "+");
						beatWidth++;
					}
					
					if (beat.effects.beatEffectFlags & gp_beatfx_tremolo_or_tap) {
						switch (beat.effects.tremoloOrTap) {
							case 0:	// tremolo bar
								wprintw(tabDisplayWindow, "v");
								beatWidth++;
								break;
							case 1:	// tap
								wprintw(tabDisplayWindow, "t");
								beatWidth++;
								break;
							case 2:	// slap
								wprintw(tabDisplayWindow, "s");
								beatWidth++;
								break;
							case 3:	// pop
								wprintw(tabDisplayWindow, "P");
								beatWidth++;
								break;
						}
					}
				}
				
				if (note.noteFlags & gp_note_has_effects) {
					if (note.noteEffectFlags & gp_notefx_bend) {
						switch (note.noteBend.type) {
							case gp_bendtype_bend:
								wprintw(tabDisplayWindow, "b");
								beatWidth++;
								break;
							case gp_bendtype_bend_release:
								wprintw(tabDisplayWindow, "br");
								beatWidth += 2;
								break;
							case gp_bendtype_bend_release_bend:
								wprintw(tabDisplayWindow, "brb");
								beatWidth += 3;
								break;
							case gp_bendtype_prebend:
								wprintw(tabDisplayWindow, "pb");
								beatWidth += 2;
								break;
							case gp_bendtype_prebend_release:
								wprintw(tabDisplayWindow, "pbr");
								beatWidth += 3;
								break;
							default:
								break;
						}
					}
					
					if (note.noteEffectFlags & gp_notefx_hammer_pull) {
						Note followingNote;
						if (beatIndex+1 < measure.beatCount) {
							followingNote = measure.beats[beatIndex + 1].beatNotes.strings[stringIndex];
						}
						else {
							followingNote = song.measures[measureIndex+1][trackIndex].beats[0].beatNotes.strings[stringIndex];
						}
						
						if (followingNote.fretNumber < note.fretNumber) {
							wprintw(tabDisplayWindow, "p");
						}
						else {
							wprintw(tabDisplayWindow, "h");
						}
						// beatWidth is not incremented, cause there shouldn't be any space before the next note
					}
					
					if (note.noteEffectFlags & gp_notefx_slide) {
						Note followingNote;
						if (beatIndex+1 < measure.beatCount) {
							followingNote = measure.beats[beatIndex + 1].beatNotes.strings[stringIndex];
						}
						else {
							followingNote = song.measures[measureIndex+1][trackIndex].beats[0].beatNotes.strings[stringIndex];
						}
						
						if (followingNote.fretNumber < note.fretNumber) {
							wprintw(tabDisplayWindow, "\\");
						}
						else {
							wprintw(tabDisplayWindow, "/");
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
		
		wmove(tabDisplayWindow, 0, beatOffset+maxBeatWidth);
		
		if (beatIndex+1 >= measure.beatCount) {	// check if end of measure reached	
			if (measureIndex+1 >= song.measureCount) {	// check if end of song reached
				// clear the rest of the tab area
				beatOffset = getcurx(tabDisplayWindow);
				std::string clearString = "|";
				clearString.append(std::string(xMax-beatOffset, ' '));
				for (int stringIndex = 0; stringIndex < track.stringCount; stringIndex++) {
					mvwprintw(tabDisplayWindow, topMargin+stringIndex, beatOffset, "%s", clearString.c_str());
				}
				wrefresh(tabDisplayWindow);
				return displayedBeats;
			}
			
			beatIndex = 0;
			measureIndex++;
			
			// measure index has changed, so get the new measure object
			measure = song.measures[measureIndex][trackIndex];
			
			// print bar line
			beatOffset = getcurx(tabDisplayWindow);
			for (int stringIndex = 0; stringIndex < track.stringCount; stringIndex++) {
				mvwprintw(tabDisplayWindow, topMargin+stringIndex, beatOffset, "|-");
			}
		}
		else {
			beatIndex++;
		}
	}
	
	wrefresh(tabDisplayWindow);
	return displayedBeats;
}

void editTab() {
	initTabDisplay();
	
	TrackHeader track = song.trackHeaders[trackIndex];
	
	int startingMeasure = 0;
	int startingBeat = 0;
	
	std::vector<DisplayedBeat> displayedBeats;
	displayedBeats = printBeats(startingMeasure, startingBeat);
	
	// allow reading non-character keypresses
	keypad(tabDisplayWindow, true);
	
	int selectionIndex = 0;
	int stringIndex = 0;
	
	while (true) {
		DisplayedBeat selectedBeat = displayedBeats[selectionIndex];
		
		// mark selected note
		char* selection = (char*)malloc(sizeof(char) * selectedBeat.beatWidth-1);
		mvwinnstr(tabDisplayWindow, stringIndex+3, selectedBeat.beatOffset, selection, selectedBeat.beatWidth-1);
		wattron(tabDisplayWindow, A_REVERSE);
		mvwprintw(tabDisplayWindow, stringIndex+3, selectedBeat.beatOffset, "%s", selection);
		wattroff(tabDisplayWindow, A_REVERSE);
		wrefresh(tabDisplayWindow);
		
		Beat beat = song.measures[selectedBeat.measureIndex][trackIndex].beats[selectedBeat.beatIndex];
		
		printBeatInfo(selectedBeat, stringIndex);
		
		keyboardInput = wgetch(tabDisplayWindow);
		
		switch (keyboardInput) {
			case KEY_LEFT:
				if (selectionIndex > 0) {
					mvwprintw(tabDisplayWindow, stringIndex+3, selectedBeat.beatOffset, "%s", selection);
					selectionIndex--;
				}
				else if (selectedBeat.beatIndex > 0) {
					startingBeat--;
					displayedBeats = printBeats(startingMeasure, startingBeat);
				}
				else if (selectedBeat.measureIndex > 0) {
					startingMeasure--;
					startingBeat = song.measures[startingMeasure][trackIndex].beatCount-1;
					displayedBeats = printBeats(startingMeasure, startingBeat);
				}
				break;
			case KEY_RIGHT:
				if ((unsigned int)selectionIndex < displayedBeats.size() - 1) {
					mvwprintw(tabDisplayWindow, stringIndex+3, selectedBeat.beatOffset, "%s", selection);
					selectionIndex++;
				}
				else if (startingBeat < song.measures[startingMeasure][trackIndex].beatCount-1) {
					startingBeat++;
					displayedBeats = printBeats(startingMeasure, startingBeat);
					selectionIndex = displayedBeats.size()-1;
				}
				else if (startingMeasure < song.measureCount-2) {
					startingMeasure++;
					startingBeat = 0;
					displayedBeats = printBeats(startingMeasure, startingBeat);
					selectionIndex = displayedBeats.size()-1;
				}
				break;
			case KEY_UP:
				if (stringIndex > 0) {
					mvwprintw(tabDisplayWindow, stringIndex+3, selectedBeat.beatOffset, "%s", selection);
					stringIndex--;
				}
				break;
			case KEY_DOWN:
				if (stringIndex < track.stringCount - 1) {
					mvwprintw(tabDisplayWindow, stringIndex+3, selectedBeat.beatOffset, "%s", selection);
					stringIndex++;
				}
				break;
			case KEY_SLEFT:
				if (startingMeasure > 0) {
					startingMeasure--;
					displayedBeats = printBeats(startingMeasure, 0);
				}
				break;
			case KEY_SRIGHT:
				if (startingMeasure < song.measureCount-2) {
					startingMeasure++;
					displayedBeats = printBeats(startingMeasure, 0);
					if ((unsigned int)selectionIndex > displayedBeats.size()-1) {
						selectionIndex = displayedBeats.size()-1;
					}
				}
				break;
		}
		if (keyboardInput == 27) {
			break;
		}
	}
	
	wclear(beatInfoWindow);
	wrefresh(beatInfoWindow);
	delwin(beatInfoWindow);
	
	wclear(tabDisplayWindow);
	wrefresh(tabDisplayWindow);
	delwin(tabDisplayWindow);
	
	refresh();
}