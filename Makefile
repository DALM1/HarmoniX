CC = gcc
CFLAGS = `pkg-config --cflags gtk+-3.0` -Iinclude
LIBS = `pkg-config --libs gtk+-3.0`
SRC = src/main.c src/ui.c
OUT = mediaflow

all: $(SRC)
	$(CC) $(CFLAGS) -o $(OUT) $(SRC) $(LIBS)

clean:
	rm -f $(OUT)
