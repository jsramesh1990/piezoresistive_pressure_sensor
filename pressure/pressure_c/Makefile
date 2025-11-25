CC = gcc
CFLAGS = -Wall -O2 `pkg-config --cflags gtk4` -I./src
LDFLAGS = `pkg-config --libs gtk4` -lpthread

SRC = src/ads1115.c src/pressure_conv.c src/gui/pressure_gui.c
OBJ = $(SRC:.c=.o)
BIN_DIR = bin
TARGET = $(BIN_DIR)/pressure_gui

all: $(TARGET)

$(TARGET): $(SRC)
	mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $(SRC) $(LDFLAGS)

clean:
	rm -rf $(BIN_DIR) *.o

.PHONY: all clean

