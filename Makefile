CC = clang
CFLAGS = -Wall -Werror -Wfatal-errors -std=c99 -g
CFLAGS_FLEX = -std=c99 -D_POSIX_SOURCE
SOURCES = $(wildcard src/*.c)
OBJECTS = $(patsubst src/%.c, build/%.o, $(SOURCES)) build/gen_lexer.o

all: build

.PHONY: build
build: dirs schc

schc: $(OBJECTS)
	$(CC) $(OBJECTS) -o $@

.PHONY: dirs
dirs:
	@mkdir -p build

.PHONY: clean
clean:
	rm -rf build schc 

build/gen_lexer.c: src/gen_lexer.l
	flex -o $@ $^

build/gen_lexer.o: build/gen_lexer.c
	$(CC) $(CFLAGS_FLEX) -o $@ -c $<

build/%.o: src/%.c
	$(CC) $(CFLAGS) -o $@ -c $<
