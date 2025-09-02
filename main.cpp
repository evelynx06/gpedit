#include <iostream>
#include <fstream>

#include "GPFile.hpp"

using namespace std;


int main(int argc, char const *argv[]) {
	string filePath = argc == 2 ? argv[1] : "test.gp3";
	
	ifstream fileStream(filePath, ios::in|ios::binary);
	if (!fileStream) {
		cerr << "Error opening file.";
		return 1;
	}
	
	
	
	fileStream.close();
	return 0;
}
