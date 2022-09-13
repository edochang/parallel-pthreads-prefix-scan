CC = g++ 
# EC 8/28 - Add a Make wildcard expansion function, so the list of files will be listed explicitely.
SRCS = $(wildcard src/*.cpp)
INC = $(wildcard src/)
OPTS = -std=c++17 -Wall -Werror -lpthread -O3

EXEC = bin/prefix_scan

all: clean compile

compile:
	$(CC) $(SRCS) $(OPTS) -I$(INC) -o $(EXEC)

clean:
	rm -f $(EXEC)
