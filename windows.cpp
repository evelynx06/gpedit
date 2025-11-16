#include "editing.hpp"
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
	// box(beatInfoWindow, 0, 0);
	// wattron(beatInfoWindow, A_REVERSE);
	// wprintw(beatInfoWindow, "Beat Info");
	// wattroff(beatInfoWindow, A_REVERSE);
	
	refresh();
	wrefresh(tabDisplayWindow);
	wrefresh(beatInfoWindow);
}

void printBeatInfo(DisplayedBeat selectedBeat, int stringIndex) {
	Beat beat = song.measures[selectedBeat.measureIndex][trackIndex].beats[selectedBeat.beatIndex];
	Note note = beat.beatNotes.strings[stringIndex];
	int line = 0;
	
	wclear(beatInfoWindow);
	
	mvwprintw(beatInfoWindow, line++, 1, "Measure: %d", selectedBeat.measureIndex+1);
	mvwprintw(beatInfoWindow, line++, 1, "Beat: %d", selectedBeat.beatIndex+1);
	mvwprintw(beatInfoWindow, line++, 1, "Rest: %s", beat.isRest ? "Yes" : "No");
	
	std::string duration;
	switch (beat.duration) {
		case gp_duration_whole:
		duration = "1/1";
		break;
		case gp_duration_half:
		duration = "1/2";
		break;
		case gp_duration_quarter:
		duration = "1/4";
		break;
		case gp_duration_eighth:
		duration = "1/8";
		break;
		case gp_duration_sixteenth:
		duration = "1/16";
		break;
		case gp_duration_thirty_second:
		duration = "1/32";
		break;
		case gp_duration_sixty_fourth:
		duration = "1/64";
		break;
	}
	mvwprintw(beatInfoWindow, line++, 1, "Duration: %s%s", duration.c_str(), (beat.beatFlags & gp_beat_is_dotted) ? "." : "");
	if (beat.beatFlags & gp_beat_is_tuplet) {
		mvwprintw(beatInfoWindow, line++, 1, "Tuplet: %d", beat.tupletDivision);
	}
	
	
	
	
	
	if (beat.beatFlags & gp_beat_has_chord) {
		mvwprintw(beatInfoWindow, line++, 1, "Chord: %s", beat.chordDiagram.name.c_str());
	}
	if (beat.beatFlags & gp_beat_has_text) {
		mvwprintw(beatInfoWindow, line++, 1, "Text: %s...", beat.text.substr(0,5).c_str());
	}
	
	
	// next column
	line = 0;
	
	if ((note.noteFlags & gp_note_has_effects) && (note.noteEffectFlags & gp_notefx_let_ring)) {
		mvwprintw(beatInfoWindow, line++, 16, "Note effects:");
		mvwprintw(beatInfoWindow, line++, 16, "\tLet ring");
	}
	
	// next column
	line = 0;
	
	if (beat.beatFlags & gp_beat_has_effects) {
		mvwprintw(beatInfoWindow, line++, 32, "Beat effects:");
		if (beat.effects.beatEffectFlags & gp_beatfx_fade_in) {
			mvwprintw(beatInfoWindow, line++, 32, "\tFade in");
		}
		if ((beat.effects.beatEffectFlags & gp_beatfx_tremolo_or_tap) && beat.effects.tremoloOrTap == 0) {
			mvwprintw(beatInfoWindow, line++, 32, "\tTrem. bar (%d st)", int(beat.effects.tremoloValue / 50));
		}
		if (beat.effects.beatEffectFlags & gp_beatfx_strum) {
			mvwprintw(beatInfoWindow, line++, 32, "\tStrum: %s", beat.effects.strumDown > 0 ? "Down" : beat.effects.strumUp > 0 ? "Up" : "None");
		}
	}
	
	wrefresh(beatInfoWindow);
}