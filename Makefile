CC = gcc
CFLAGS = -Wall -g
LIBS = -lSDL2

SOURCES = main.c cpu.c emu8.c opcodes.c keyboard.c
OBJECTS = $(SOURCES:.c=.o)
EXEC = emu8

all: $(EXEC)

$(EXEC): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(EXEC) $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(EXEC)