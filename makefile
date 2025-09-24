all: gpedit

gpedit: main.cpp
	g++ main.cpp -o gpedit

run: all
	./gpedit