# Makefile for Gake.  This is terrible, but it's better than the previous one.  These things are incomprehensible magic.
#
# This is designed to be used with GNU Make.  Please do not take this as praise of the FSF:  it's just what I had.
#
# The comments of this file assume that you understand makefiles.  If you don't, read the wikipedia page, then use the GNU Make reference manual _as-needed_.
# 	· Wikipedia page:  https://en.wikipedia.org/wiki/Make_(software)
# 	· GNU Make Manual:  https://www.gnu.org/software/make/manual/
#
# Oh, also, this assumes knowledge of ANSI escape sequences.  You can just use the Wikipedia page; the official standard is a nightmare to navigate and contains a lot of pointless boilerplate.
# 	· Wikipedia page:  https://en.wikipedia.org/wiki/ANSI_escape_code
# 	· ECMA-48 (do not use:)  https://www.ecma-international.org/wp-content/uploads/ECMA-48_5th_edition_june_1991.pdf

ifneq (,)
This makefile is designed for GNU Make.
endif

CC = clang-13
CFLAGS = -Wall -Werror -Wextra -std=c2x -fdiagnostics-show-category=name ` sdl2-config --cflags ` -O2 # `-pedantic` should probably also be here, but I couldn't figure out how to include everything in it _except_ the thing preventing `\e from being used as an escape sequence for the escape character.
CFLAGS_DEBUG = -Wall -Werror -Wextra -std=c2x -fdiagnostics-show-category=name ` sdl2-config --cflags ` -O0 -glldb
LDFLAGS = ` sdl2-config --libs ` -lz
# I don't know what this does or how it works.  I stole this from the makefile for jdh's 48-hour Minecraft clone.
SRC = $(wildcard Source/*.c)
OBJ = $(SRC:.c=.o)

help:
	@echo "No target selected.  Available targets are:\n"\
	"	help: Show this blurb and exit.\n"\
	"	release: Prepare a release build.\n"\
	"	debug: Prepare a debug build.\n"\
	"\\e[41m\\e[1m**DANGER ZONE**\\e[m\n"\
	"	\\e[31minstall: Installs the software and associated items.\n"\
	"	clean: Cleans out object files and binaries.\\e[m\n"

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Compiling a debug target works by recompiling the Main file with a macro `GAKE_DEBUG` defined.  The headers included in the Main file check to see if this macro is defined, and depending on whether it is, expose or don't expose certain functions.  This means that to make a debug build, _only_ the main file needs to be recompiled, instead of all of the files.  (Did that make sense?  I sure hope so.)

release: $(OBJ)
	$(CC) $(CFLAGS) -c Source/Main.c -o Source/Main.o
	$(CC) $(LDFLAGS) $^ -o Gake.elf

debug: $(OBJ)
	$(CC) $(CFLAGS_DEBUG) -DGAKE_DEBUG -c Source/Main.c -o Source/Main.o
	$(CC) $(LDFLAGS) $^ -o Gake.elf

# I intend to implement installation of the API once it exists.  Also, the asset copying method is really stupid.  I'm aware that this checks if the directory exists every time, but I think that the time benefit from restructuring it to not do that would be too small to be useful.
install: Gake.elf
	@if [ $$USER = root ] ; then\
		cp Gake.elf /usr/local/games/gake ;\
		if [ ! -e /usr/local/share/Gake/ ] ;\
			then mkdir -p /usr/local/share/Gake/Assets/ ; fi ;\
		cp -r Assets/*.png /usr/local/share/Gake/Assets/ ;\
		if [ ! -e /usr/local/man/man6/ ] ;\
			then mkdir -p /usr/local/man/man6/ ; fi ;\
		cp Documentation/gake.6 /usr/local/man/man6/gake.6 ;\
		gzip /usr/local/man/man6/gake.6 ; \
	else echo "You can only install Gake as root!" ; fi

clean: $(OBJ)
	rm $^
	if [ -e Gake.elf ] ; then rm Gake.elf ; fi
