ifeq ($(OS), Windows_NT)
	CURSESLIB := pdcurses
else
	CURSESLIB := ncurses
endif

BUILD_DIR = build
OBJ_DIR = $(BUILD_DIR)/obj

OBJS = $(OBJ_DIR)/editing.o \
		 $(OBJ_DIR)/gp_file.o \
		 $(OBJ_DIR)/gp_read.o \
		 $(OBJ_DIR)/gpedit.o \
		 $(OBJ_DIR)/main.o \
		 $(OBJ_DIR)/windows.o
		 
LIBS = -l$(CURSESLIB)
CFLAGS = -Wall
EXEC = $(BUILD_DIR)/gpedit


.PHONY: all clean

all: $(EXEC)

clean:
	rm -r $(BUILD_DIR)

$(EXEC): $(OBJS)
	@mkdir -p $(BUILD_DIR)
	g++ $(OBJS) $(LIBS) -o $(EXEC)

$(OBJ_DIR)/%.o: %.cpp
	@mkdir -p $(OBJ_DIR)
	g++ $(CFLAGS) -c -o $@ $<

$(OBJ_DIR)/editing.o: editing.cpp editing.hpp gpedit.hpp gp_file.hpp windows.hpp
$(OBJ_DIR)/gp_file.o: gp_file.cpp gp_file.hpp gp_read.hpp
$(OBJ_DIR)/gp_read.o: gp_read.cpp gp_read.hpp
$(OBJ_DIR)/gpedit.: gpedit.cpp gpedit.hpp gp_file.hpp
$(OBJ_DIR)/main.o: main.cpp gpedit.hpp windows.hpp editing.hpp
$(OBJ_DIR)/windows.o: windows.cpp windows.hpp gpedit.hpp gp_file.hpp