CC = gcc
CFLAGS = -Wall -Wextra -std=c17 -I./src
CLIBS = -larchive

TARGET = output/spk

SRC = $(wildcard src/*.c) \
      $(wildcard src/commands/*.c) \
      $(wildcard src/package/*.c) \
	  $(wildcard src/tar/*.c)

OBJ = $(SRC:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJ)
	@mkdir -p output
	$(CC) $(OBJ) -o $@ $(CLIBS)

clean:
	rm -rf output/*
	rm -f src/*.o
	rm -f src/*/*.o