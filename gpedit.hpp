#ifndef GPEDIT_H
#define GPEDIT_H

#include "gp_file.hpp"

extern GPFile song;
extern std::string songFilePath;

extern int keyboardInput;

extern int trackIndex;

int openFile(std::string filePath);

#endif // !GPEDIT_H