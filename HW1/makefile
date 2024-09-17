include src/code/targets.mk
IGNORED_WARNINGS=-Wno-unused-result -Wno-uninitialized -Wno-char-subscripts
all:
	gcc -std=gnu99 -O2 -o run src/main.c src/lib/input_parser.c $(TARGETS) -Wall -Werror $(IGNORED_WARNINGS)
