#include <iostream>
#include <fstream>

#include <bitset>

#include "GPFile.hpp"

int print_beat(GPFile song, int trackIndex, int measureIndex, int beatIndex) {
	TrackHeader track = song.trackHeaders[trackIndex];
	Beat beat = song.measures[measureIndex][trackIndex].beats[beatIndex];

	std::cout << "Song: " << song.metadata.title << "\n";
	std::cout << "Track: #" << trackIndex+1 << " (" << track.name << ")\n";
	std::cout << "Measure: #" << measureIndex+1 << "\n";
	std::cout << "Beat: #" << beatIndex+1 << "\n";
	
	std::cout << "Note division: " << (1 << (beat.duration + 2)) << "\n";
	
	std::cout << "Notes:\n";
	for (int i = 0; i < track.stringCount; i++) {
		std::cout << "\t[" << track.stringTuning[i] << "] |-";
		if (beat.beatNotes.stringsPlayed & (0x40 >> i)) {
			Note note = beat.beatNotes.strings[i];
			
			if (note.noteFlags & gp_note_is_ghost) {
				std::cout << "(";
			}
			
			if (note.noteType == gp_note_type_dead) {
				std::cout << "x";
			}
			else {
				std::cout << (int)note.fretNumber;
				if (note.noteType == gp_note_type_tied) {
					std::cout << "*";
				}
			}
			
			if (note.noteFlags & gp_note_is_ghost) {
				std::cout << ")";
			}
		}
		else {
			std::cout << "-";
		}
		
		std::cout << "-|\n";
	}
	
	return 0;
}

int main(int argc, char const *argv[]) {
	std::string filePath = argc == 2 ? argv[1] : "test.gp3";
	
	std::ifstream fileStream(filePath, std::ios::in|std::ios::binary);
	if (!fileStream) {
		std::cerr << "Error opening file.\n";
		return 1;
	}
	
	GPFile song;
	
	song.read_song(fileStream);
	
	fileStream.close();
	
	int trackIndex = 0;
	int measureIndex = 0;
	int beatIndex = 0;
	
	std::cout << "Input track number (Opened file has " << song.trackCount << " track(s)):\n\t";
	std::cin >> trackIndex;
	trackIndex--;
	
	while (true) {
		std::cout << "\n----------------------\n\n";
		print_beat(song, trackIndex, measureIndex, beatIndex);
		std::cout << "\n----------------------\n\n";
		
		std::string command;
		std::cout << "Input command:\n\t";
		std::cin >> command;
		
		if (command == "next" || command == "n") {
			if (beatIndex + 1 >= song.measures[measureIndex][trackIndex].beatCount) {
				if (measureIndex + 1 >= song.measureCount) {
					std::cout << "End of song reached!\n";
					break;
				}
				beatIndex = 0;
				measureIndex++;
			}
			else {
				beatIndex++;
			}
		}
		else if (command == "prev" || command == "p") {
			if (beatIndex <= 0) {
				if (measureIndex <= 0) {
					std::cout << "Beginning of song reached!\n";
					break;
				}
				measureIndex--;
				beatIndex = song.measures[measureIndex][trackIndex].beatCount - 1;
			}
			else {
				beatIndex--;
			}
		}
	}
	
	return 0;
}
