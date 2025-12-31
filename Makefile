CC= clang
IFLAGS= -I /opt/homebrew/include
LFLAGS= -L /opt/homebrew/lib -lSDL3
CFLAGS= -Wall -Wextra
SOURCES= main.c memory.c debug.c display.c interpret.c
OBJECTS= $(SOURCES:.c=.o)

all: main

main: $(OBJECTS)
	$(CC) $(IFLAGS) $(LFLAGS) $(CFLAGS) $^ -o $@

%.o: %.c %.h
	$(CC) $(CFLAGS) $(IFLAGS) $< -c -o $@

%.o: %.c
	$(CC) $(CFLAGS) $(IFLAGS) $< -c -o $@

clean:
	rm -f $(OBJECTS)
	rm -f main
