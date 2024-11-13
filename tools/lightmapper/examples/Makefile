CC = g++
STD = -std=c++20
COMPILER_FLAGS = -Wall -O0 -Wfatal-errors
INCLUDE_PATH = -I./../../../ \
			   -I ./../../../dependencies/SDL2.28.4/include \
			   -I ./../../../dependencies/

SRC_FILES = read_lightmapper_binary_usage.cpp \
			./../../../map_parser.h \
			./../../../dependencies/stb_image.h \
			./../../../polysoup.cpp \
			./../../../image.cpp \
			./../../../platform.cpp 

LINKER_FLAGS = -lSDL2
OUT = read_lightmapper_bin 

build:
	$(CC) $(COMPILER_FLAGS) $(STD)  -o $(OUT) $(INCLUDE_PATH) $(SRC_FILES) $(LINKER_FLAGS);

run:
	./$(OUT)

clean:
	rm $(OUT)

