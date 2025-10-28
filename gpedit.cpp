#include <iostream>
#include <fstream>

#include "gpedit.hpp"
#include "gp_file.hpp"

GPFile song;
std::string songFilePath;

int keyboardInput;

int trackIndex = 0;

int openFile(std::string filePath) {
	songFilePath = filePath;
	// open file
	std::ifstream fileStream(filePath, std::ios::in|std::ios::binary);
	if (!fileStream) {
		std::cerr << "Error opening file.\n";
		return 1;
	}
	
	// read file
	song.read_song(fileStream);
	
	fileStream.close();
	
	return 0;
}