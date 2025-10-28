#ifndef GP_READ_H
#define GP_READ_H

#include <fstream>

namespace gp_read
{
	unsigned char read_byte(std::ifstream &fileStream);
	char read_signedbyte(std::ifstream &fileStream);
	bool read_bool(std::ifstream &fileStream);
	short read_short(std::ifstream &fileStream);
	int read_int(std::ifstream &fileStream);
	std::string read_bytestring(std::ifstream &fileStream);
	std::string read_intstring(std::ifstream &fileStream);
	std::string read_intbytestring(std::ifstream &fileStream);
};

#endif // !GP_READ_H