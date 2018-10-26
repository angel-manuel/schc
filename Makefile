CC = clang
CFLAGS = -Wall -Werror -Wfatal-errors -std=c99 -Isrc
CFLAGS_FLEX = -std=c99 -D_POSIX_SOURCE
SOURCES = $(filter-out src/schc.c, $(wildcard src/*.c src/**/*.c))
OBJECTS = $(patsubst src/%.c, build/%.o, $(SOURCES)) build/gen_lexer.o
TESTS = $(patsubst tests/%.c, %-test, $(wildcard tests/*.c))

.PHONY: all
all: debug

.PHONY: debug
debug: CFLAGS += -DDEBUG -g
debug: build

.PHONY: release
release: CFLAGS += -DNDEBUG -O3
release: build

.PHONY: build
build: dirs schc

.PHONY: tests
tests: CFLAGS += -DDEBUG -g
tests: dirs $(TESTS)

schc: $(OBJECTS) build/schc.o
	$(CC) $^ -o $@

%-test: $(OBJECTS) build/%-test.o
	$(CC) $^ -o $@

.PHONY: dirs
dirs:
	@mkdir -p build

.PHONY: clean
clean:
	rm -rf build schc $(TESTS)

build/gen_lexer.c: src/gen_lexer.l
	flex -o $@ $^

build/gen_lexer.o: build/gen_lexer.c
	$(CC) $(CFLAGS_FLEX) -o $@ -c $<

build/%-test.o: tests/%.c src/test.h
	$(CC) $(CFLAGS) -o $@ -c $<

build/%/:
	mkdir -p $@

build/%.o: src/%.c
	$(CC) $(CFLAGS) -o $@ -c $<

$(foreach OBJECT, $(OBJECTS), $(eval $(OBJECT): | $(dir $(OBJECT))))
