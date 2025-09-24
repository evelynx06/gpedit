all: gpedit

gpedit: main.cpp
	g++ main.cpp -lncurses -o ./build/gpedit