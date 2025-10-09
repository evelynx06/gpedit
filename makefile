ifeq ($(OS), Windows_NT)
	curseslib := pdcurses
else
	curseslib := ncurses
endif

all: gpedit

gpedit: main.cpp
	g++ -g main.cpp -l$(curseslib) -o ./build/gpedit