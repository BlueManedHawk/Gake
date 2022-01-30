# Makefile for Gake.  Some notes:
# 	· Most things ending in `_r` are for release builds; most things ending in `_d` are for debug builds.
# 	· This builds the object files for debug and release builds in tandem.  While this preserves the nice benefit of "only rebuilding files when needed", it does make it take up twice as much space, and duplicates files that have no difference between the release and debug builds.  Still, this is better than some of the previous iterations.
# 	· This is designed to be used with GNU Make.  Please do not take this as praise of the FSF:  it's just what I had access to.
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

SHELL = /bin/bash
CC = clang-13
CFLAGS = -Wall -Werror -Wextra -std=c2x -fdiagnostics-show-category=name ` sdl2-config --cflags ` # `-pedantic` should probably also be here, but I couldn't figure out how to include everything in it _except_ the thing preventing `\e from being used as an escape sequence for the escape character.
CFLAGS_R = -O3
CFLAGS_D = -O0 -DGAKE_DEBUG -glldb
LDFLAGS = ` sdl2-config --libs ` -lz -ldl
# Might be possible to make this less repetitive.
GME_SRC = $(wildcard Source/*.c)
API_SRC = $(wildcard API/*.c)
GME_OBJ_R = $(GME_SRC:.c=_r.o)
GME_OBJ_D = $(GME_SRC:.c=_d.o)
API_OBJ_R = $(API_SRC:.c=_r.o)
API_OBJ_D = $(API_SRC:.c=_d.o)

help:
	@sh -c 'echo "Available targets for Gake are:\n"\
	"	help: Show this blurb and exit.\n"\
	"	release: Prepare a release build.\n"\
	"	debug: Prepare a debug build.\n"\
	"\\e[41m\\e[1m**DANGER ZONE**\\e[m\n"\
	"	\\e[31minstall: Installs the software and associated items.\n"\
	"	clean: Cleans out object files and binaries.\\e[m\n"\
	"\n"\
	"\\e[1mThese are the only targets that should be called.\\e[m"'

# This ain't perfect.  TODO: see if it's possible to make this better.

%_r.o: %.c
	$(CC) $(CFLAGS) $(CFLAGS_R) -c $< -o $@

%_d.o: %.c
	$(CC) $(CFLAGS) $(CFLAGS_D) -c $< -o $@

release: _release_game _release_api

_release_game: $(GME_OBJ_R)
	$(CC) $(LDFLAGS) $^ -o Gake.elf

_release_api: $(API_OBJ_R)
	$(CC) $(LDFLAGS) $^ -o libgake.so

debug: _debug_game _debug_api

_debug_game: $(GME_OBJ_D)
	$(CC) $(LDFLAGS) $^ -o Gake.elf

_debug_api: $(API_OBJ_D)
	$(CC) $(LDFLAGS) $^ -o libgake.so

# I'm aware that this checks if the directories exists every time, but I think that the time benefit from restructuring it to not do that would be too small to be useful.  Also, yeah, it would be nice to simplify the manpage installation process, but since there aren't too many manpages right now, I think that can wait.

install: Gake.elf _install_manpages #libgake.so
	@if [ $$USER = root ] ; then\
		cp Gake.elf /usr/local/games/gake ;\
		cp libgake.so /usr/local/lib/libgake.so ;\
		cp ./API/Gake.h /usr/local/include/Gake.h ;\
		if [ ! -e /usr/local/share/Gake/ ] ;\
			then mkdir -p /usr/local/share/Gake/Assets/ ; fi ;\
		cp -r Assets/*.png /usr/local/share/Gake/Assets/ ;\
		cp -r Assets/*.txt /usr/local/share/Gake/Assets/ ;\
	else echo "You can only install Gake as root!" ; fi

_install_manpages: _install_man7pages
	@if [ $$USER = root ] ; then\
		if [ ! -e /usr/local/man/man6/ ] ;\
			then mkdir -p /usr/local/man/man6/ ; fi ;\
		cp Documentation/gake.6 /usr/local/man/man6/gake.6 ;\
		gzip -9f /usr/local/man/man6/gake.6 ;\
	fi

_install_man7pages: $(wildcard Documentation/*.7)
	@if [ $$USER = root ] ; then\
		if [ ! -e /usr/local/man/man7/ ] ;\
			then mkdir -p /usr/local/man/man7/ ; fi ;\
		cp $^ /usr/local/man/man7/ ; \
		gzip -9f $(subst Documentation/,/usr/local/man/man7/,$^) ; \
	fi

clean: $(GME_OBJ_R) $(GME_OBJ_D) $(API_OBJ_R) $(API_OBJ_D)
	rm $^
	if [ -e Gake.elf ] ; then rm Gake.elf ; fi
