# Makefile for Gake.
#
# There is no `clean` or `install` target because I don't trust myself.
#
# This is designed to use GNU Make, not because I like it or condone the FSF, but because it's the only Make I have access to.  This may not work on other makes.

# Supposedly, this will fail on non-GNU Makes.  I can't test that, since I don't have access to any, so I'll just hope this works.
ifneq (,)
You need to use GNU Make in order for this to work properly.
endif

SRC := $(wildcard Source/*.c)
OBJ := $(SRC:.c=.o)
CC := clang-13 # Some reliance on Clang-specific stuff is done.
CFLAGS := -std=c2x -glldb -g -O3 -Wall -Werror -Wextra -pedantic `sdl2-config --cflags` -fdiagnostics-show-category=name # There's something stupid about this and I don't know what.
LDFLAGS := `sdl2-config --libs` -v

.PHONY: default release debug install # What does this do again?

default:
	@echo "No target given, so nothing has been done.  Options are:\n"\
		"	default:  Display this and exit.\n"\
		"	release:  Create a normal binary.\n"\
		"	debug:  Create a debug binary.\n"\
		"There is no clean or install target because I don't trust myself."

release: $(OBJ)
	$(CC) -o Gake.elf $^ $(LDFLAGS)

debug: $(OBJ)
	$(CC) -o Gake.elf $^ Debug.o $(LDFLAGS)

%.o: %.c
	$(CC) -o $@ -c $< $(CFLAGS)
