#include <iostream>
#include <fstream>

#include "gp_read.hpp"

namespace gp_read {
	unsigned char _read_byte(std::ifstream &fileStream) {
		char buffer[1];
		fileStream.read(buffer, sizeof(buffer));
		return (unsigned char)buffer[0];
	}
	
	char _read_signedbyte(std::ifstream &fileStream) {
		char buffer[1];
		fileStream.read(buffer, sizeof(buffer));
		return buffer[0];
	}
	
	bool _read_bool(std::ifstream &fileStream) {
		char buffer[1];
		fileStream.read(buffer, sizeof(buffer));
		return (bool)buffer[0];
	}
	
	short _read_short(std::ifstream &fileStream) {
		char buffer[2];
		fileStream.read(buffer, sizeof(buffer));
		return (unsigned char)(buffer[0]) | (unsigned char)(buffer[1]) << 8;
	}
	
	int _read_int(std::ifstream &fileStream) {
		char buffer[4];
		fileStream.read(buffer, sizeof(buffer));
		return (unsigned char)(buffer[0]) | (unsigned char)(buffer[1]) << 8 |
				 (unsigned char)(buffer[2]) << 16 | (unsigned char)(buffer[3]) << 24;
	}
	
	std::string _read_bytestring(std::ifstream &fileStream) {
		int length = read_byte(fileStream);
		char buffer[length + 1];
		fileStream.read(buffer, length);
		buffer[length] = 0; // null termination
		return buffer;
	}
	
	std::string _read_intstring(std::ifstream &fileStream) {
		int length = read_int(fileStream);
		char buffer[length + 1];
		fileStream.read(buffer, length);
		buffer[length] = 0; // null termination
		return buffer;
	}
	
	std::string _read_intbytestring(std::ifstream &fileStream) {
		int lengthInt = read_int(fileStream);
		int lengthByte = read_byte(fileStream);

		if (lengthInt != lengthByte + 1) {
			std::cerr << "Error: Mismatched string lengths in IntByteString.\n";
			return "";
		}

		char buffer[lengthByte + 1];
		fileStream.read(buffer, lengthByte);
		buffer[lengthByte] = 0; // null termination
		return buffer;
	}
}