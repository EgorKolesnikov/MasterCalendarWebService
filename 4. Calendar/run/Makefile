CC=g++
LIBS=-lcppcms -lbooster -lmongocxx -lbsoncxx
CFLAGS=-c -Wall -std=c++11 -L/usr/local/lib -I/usr/local/include/mongocxx/v_noabi -I/usr/local/include/bsoncxx/v_noabi -L/usr/local/lib
SRC_DIR = ../src
HEADERS = $(SRC_DIR)/util.h $(SRC_DIR)/urls.h
SOURCES = $(SRC_DIR)/util.cpp $(SRC_DIR)/urls.cpp $(SRC_DIR)/server.cpp
OBJECTS = $(SOURCES:.cpp=.o)
EXECUTABLE = server_calendar

# $(pkg-config --cflags --libs libmongocxx)


all: $(SOURCES) $(EXECUTABLE) $(HEADERS)
	
$(EXECUTABLE): $(OBJECTS) $(HEADERS)
	$(CC) $(OBJECTS) -o $(EXECUTABLE) $(LIBS)

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@ $(LIBS)

clean:
	rm -rf $(EXECUTABLE) $(OBJECTS)
