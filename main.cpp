#include <iostream>
#include <fstream>

using namespace std;

unsigned char gp_read_byte(ifstream &fileStream) {
	char buffer[1];
	fileStream.read(buffer, sizeof(buffer));
	return (unsigned char)buffer[0];
}

char gp_read_signedbyte(ifstream &fileStream) {
	char buffer[1];
	fileStream.read(buffer, sizeof(buffer));
	return buffer[0];
}

bool gp_read_bool(ifstream &fileStream) {
	char buffer[1];
	fileStream.read(buffer, sizeof(buffer));
	return (bool)buffer[0];
}

unsigned short gp_read_short(ifstream &fileStream) {
	char buffer[2];
	fileStream.read(buffer, sizeof(buffer));
	return (unsigned char)(buffer[0]) |
			 (unsigned char)(buffer[1]) << 8;
}

unsigned int gp_read_int(ifstream &fileStream) {
	char buffer[4];
	fileStream.read(buffer, sizeof(buffer));
	return (unsigned char)(buffer[0]) |
			 (unsigned char)(buffer[1]) << 8 |
			 (unsigned char)(buffer[2]) << 16 |
			 (unsigned char)(buffer[3]) << 24;
}

string gp_read_bytestring(ifstream &fileStream) {
	size_t length = gp_read_byte(fileStream);
	char buffer[length + 1];
	fileStream.read(buffer, length);
	buffer[length] = 0;		// null termination
	return buffer;
}

string gp_read_intstring(ifstream &fileStream) {
	size_t length = gp_read_int(fileStream);
	char buffer[length + 1];
	fileStream.read(buffer, length);
	buffer[length] = 0;		// null termination
	return buffer;
}

string gp_read_intbytestring(ifstream &fileStream) {
	size_t lengthInt = gp_read_int(fileStream);
	size_t lengthByte = gp_read_byte(fileStream);
	
	if (lengthInt != lengthByte + 1) {
		cerr << "Error: Mismatched string lengths in IntByteString.";
		return "";
	}
	
	char buffer[lengthByte + 1];
	fileStream.read(buffer, lengthByte);
	buffer[lengthByte] = 0;		// null termination
	return buffer;
}

int main(int argc, char const *argv[]) {
	string filePath = argc == 2 ? argv[1] : "test.gp3";
	
	ifstream fileStream(filePath, ios::in|ios::binary);
	if (!fileStream) {
		cerr << "Error opening file.";
		return 1;
	}
	
	string version = gp_read_bytestring(fileStream);
	fileStream.seekg(30 - version.length(), ifstream::cur);
	
	if (version != "FICHIER GUITAR PRO v3.00") {
		cerr << "Incompatible file format '" << version << "'" << endl;
		return 1;
	}
	
	cout << "File format: " << version << endl;
	cout << "Title: " << gp_read_intbytestring(fileStream) << endl;
	cout << "Subtitle: " << gp_read_intbytestring(fileStream) << endl;
	cout << "Artist: " << gp_read_intbytestring(fileStream) << endl;
	cout << "Album: " << gp_read_intbytestring(fileStream) << endl;
	
	fileStream.close();
	return 0;
}
