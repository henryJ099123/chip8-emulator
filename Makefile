CC= clang
IFLAGS= -I /opt/homebrew/include -I include/
LFLAGS= -L /opt/homebrew/lib -lSDL3
CFLAGS= -Wall -Wextra -Wpedantic
COMMON= include/settings.h
SOURCES= chip8.c memory.c debug.c display.c interpret.c
EXEC= chip8
OBJECTS= $(SOURCES:%.c=build/%.o)

all: $(EXEC)

$(EXEC): $(OBJECTS)
	$(CC) $(IFLAGS) $(LFLAGS) $(CFLAGS) $^ -o $@

build/%.o: src/%.c include/%.h $(COMMON) | build/
	$(CC) $(CFLAGS) $(IFLAGS) $< -c -o $@

build/%.o: src/%.c $(COMMON) | build/
	$(CC) $(CFLAGS) $(IFLAGS) $< -c -o $@

build/:
	mkdir -p build

clean:
	rm -f $(EXEC)
	rm -rf build
