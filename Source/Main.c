/* LICENSE
 *
 * Copyright © 2021 Blue-Maned_Hawk.  All rights reserved.
 *
 * This software should have come with a file called LICENSE.  In case of any difference between this comment and that file, that file is the authority.  (If you did not recieve that file, it's a violation of the license.  Please report it to me.)
 *
 * This project is copylefted.  You may freely use, distribute, and modify this software, to the extent permitted by law, so long as you do not attempt to claim such activities are condoned by the author, you distribute the license file with any distributions of this software, you release any modifications under a similar license, and you do not attempt to claim that modified software is the original software.
 *
 * This license does not apply to software created with the API of this software (thought it does apply to the API itself); it also does not apply to any rule files, all of which must be placed in the public domain.
 *
 * This software links to zlib, which is under the zlib license, available at https://www.zlib.net/zlib_license.html.
 *
 * This software dynamically links to SDL2, which is under a separate instance of the zlib license, available at https://libsdl.org/license.php.
 *
 * This software dynamically links to libgcrypt, which is under the GNU LGPL2.1+, available at https://git.gnupg.org/cgi-bin/gitweb.cgi?p=gnupg.git;a=blob;f=COPYING;h=ccbbaf61b794c7aaea10dffb486095fdc8f3a44a;hb=HEAD.
 *
 * This license does not apply to trademarks or patents.
 *
 * THIS PRODUCT COMES WITH ABSOLUTELY NO WARRANTY, IMPLIED OR EXPLICIT, TO THE EXTENT PERMITTED BY LAW.  THE AUTHOR DISCLAIMS ANY LIABILITY FOR ANY DAMAGES OF ANY KIND CAUSED BY THIS PRODUCT, TO THE EXTENT PERMITTED BY LAW.*/

/* This is the main file for Gake.  It contains the `main()` function.  I've tried my best to keep it clean.
 *
 * When reading this file, you are expected to have access to and generally understand the following documents:
 * 	· Latest draft of C2x:  http://www.open-std.org/JTC1/SC22/WG14/www/docs/n2596.pdf
 * 	· The Clang compiler user(?) manual:  https://clang.llvm.org/docs/UsersManual.html
 * 	· The latest POSIX specification:  https://pubs.opengroup.org/onlinepubs/9699919799/mindex.html */

#ifdef __cplusplus
#	error "\e[5m\e[1m\e[31mTHIS IS NOT A C++ PROGRAM."
#endif

#if __clang_major__ < 13
#	error "This program is intended to be compiled with Clang 13, which you appear to not have.  Please go obtain a copy of it."
#endif

#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <unistd.h>
#include "Crash.h"
#include <signal.h>
#include "Logging.h" /* Theoretically, this program could use POSIX's logging systems, but those seemed to me to be too implementation-defined to be very useful. */
#include <string.h>
#include <stdlib.h>
#include "Debug.h"
#include "Checks.h"
#include <dlfcn.h>
#include "SDL.h"

static const int screenwidth = 640;
static const int screenheight = 480;

struct newstate {
	_Bool grid[4][4];
};

struct curstate {
	_Bool grid[4][4];
	long long frame;
	char keys[];
};

int main(int argc, char ** argv)
{

	const struct sigaction act = {
		.sa_sigaction = handler,
		.sa_mask = { 0 },
		.sa_flags = SA_SIGINFO
	};
	const struct sigaction ign = {
		.sa_handler = SIG_IGN
	};
	const int sigs_to_handle[16] = {SIGABRT, SIGBUS, SIGFPE, SIGHUP, SIGILL, SIGINT, SIGQUIT, SIGSEGV, SIGTERM, SIGUSR1, SIGSYS, 0};
	const int sigs_to_ign[8] = {SIGALRM, SIGPIPE, SIGUSR2, SIGVTALRM, 0};
	for (register unsigned int i = 0; sigs_to_handle[i] != 0; i++){
		sigaction(sigs_to_handle[i], &act, NULL);
	}
	for (register unsigned int i = 0; sigs_to_ign[i] != 0; i++){
		sigaction(sigs_to_ign[i], &ign, NULL);
	}

	short gpcount = 0;
	/* It's really silly that I need to do all of this to get a dynamically-stored array of arrays. */
	char ** prgm_names = calloc(8, sizeof (char *));
	for (register size_t i = 0; i < ((sizeof prgm_names) / (sizeof (char *))); i++)
		prgm_names[i] = calloc(256, sizeof (char));
	_Bool * too_many = calloc(1, sizeof (_Bool));
	_Bool * nonprgms = calloc(1, sizeof (_Bool));
	for (signed char opts = 0; opts != -1; opts = getopt(argc, argv, "?hv-il:")){
		switch (opts){
		case 0:
			break;
		case '-':
			printf(
			"\e[31m\e[1mGake does not and never will support GNU-style options.\e[m\n"
			"\n"
			"(If you were trying to use \e[4m--help\e[m or \e[4m--version\e[m, please use \e[4m-h\e[m or \e[4m-v\e[m.)\n");
			free(too_many);
			free(nonprgms);
			for (register size_t i = 0; i < ((sizeof prgm_names) / (sizeof (char *))); i++)
				free(prgm_names[i]);
			free(prgm_names);
			crash(2, "No extra info.");
		case '?':
		case 'h':
			printf(
			"Gake is an open-source reimplementation of Google Snake, with extensions.\n"
			"\n"
			"Options available:\n"
			"\t\e[1m-v\e[m: \tdisplay the version.\n"
			"\t\e[1m-h\e[m or \e[1m-?\e[m: \tdisplay this help blurb.\n"
			"\t\e[1m-l\e[m \e[4m<API-using–program>\e[m: \tload the program for usage.  Up to 8 programs can be loaded at a time, though overusage of computing resources can lead to crashing.\n"
			"\n"
			"For more information, please see the manpage (available with \e[1mman gake\e[m, if installed).\n"
			"\n"
			"\e[1mThis program does not and never will support GNU-style options.\e[m\n");
			free(too_many);
			free(nonprgms);
			for (register size_t i = 0; i < ((sizeof prgm_names) / (sizeof (char *))); i++)
				free(prgm_names[i]);
			free(prgm_names);
			return 1; /* Counted as a failure for consistency with other software and not breaking things like `make`. */
		case 'v':
			printf("This is Gake vN.0, semantic version 0.0.0.\n");
			free(too_many);
			free(nonprgms);
			for (register size_t i = 0; i < ((sizeof prgm_names) / (sizeof (char *))); i++)
				free(prgm_names[i]);
			free(prgm_names);
			return 1; /* See above comment. */
		case 'l':
			if (gpcount >= 8){
				*too_many = 1;
			} else {
				void * table = dlopen(optarg, RTLD_LAZY | RTLD_LOCAL);
				if (table != NULL){
					struct newstate (*gake_main)(struct curstate) = dlsym(table, "gake_main");
					if (gake_main == NULL){
						*nonprgms = 1;
					} else {
						strcpy(prgm_names[gpcount], optarg);
						gpcount++;
					}
					dlclose(table);
				} else {
					*nonprgms = 1;
				}
			}
			break;
		}
	}

	setup_logging();

	logmsg(lp_info, lc_misc, "Gake has been started!");
	logmsg(lp_info, lc_misc, "This is Gake version N.0, semantic version 0.0.0, compiled on %s at %s.", __DATE__, __TIME__);

	debug_notice();

	logmsg(lp_debug, lc_checks, "Beginning checks…");
	[[maybe_unused]] _Bool battery_checks = 0;
	switch (run_checks()){
	case -1:
		battery_checks = 1;
	case 0:
		break;
	case 1:
		crash(0xC, "See the game log for details.");
		break;
	case 2:
		crash(0xD, "No other details.");
	}
	logmsg(lp_info, lc_checks, "All checks have passed!  Continuing as normal…");

	if (*nonprgms){
		if (gpcount <= 0)
			logmsg(lp_err, lc_api, "None of the programs you requested could be loaded.  This may be due to nonexistence or innadequacy.");
		else
			logmsg(lp_err, lc_api, "Some programs could not be loaded.  This may be due to nonexistence or innadequacy.");
	}
	free(nonprgms);
	if (*too_many)
		logmsg(lp_err, lc_api, "You have requested too many programs to be loaded.  Additional programs have been ignored.");
	free(too_many);

	struct newstate (*programs[8])(struct curstate);
	void * tables[8];
	if (gpcount >= 1){
		logmsg(lp_info, lc_api, "Loading programs…");
		for (register short i = 0; i < gpcount; i++){
			tables[i] = dlopen(prgm_names[i], RTLD_NOW | RTLD_LOCAL);
			programs[i] = dlsym(tables[i], "gake_main");
		}
		logmsg(lp_info, lc_api, "All programs have been loaded!");
	}
	for (register size_t i = 0; i < ((sizeof prgm_names) / (sizeof (char *))); i++)
		free(prgm_names[i]);
	free(prgm_names);

	SDL_Init(SDL_INIT_VIDEO);
	SDL_Window * window = SDL_CreateWindow("Gake", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, screenwidth, screenheight, 0);
	SDL_Renderer * renderer = SDL_CreateRenderer(window, -1, 0);

	SDL_Event event;
	_Bool quit = 0;
	struct {
		int x;
		int y;
		_Bool unhandled;
	} click = {
		.x = 0,
		.y = 0,
		.unhandled = 0
	};
	_Bool squares[4][4] = {
		{0, 0, 0, 0},
		{0, 0, 0, 0},
		{0, 0, 0, 0},
		{0, 0, 0, 0}
	};
	SDL_Rect rect = {
		.w = screenwidth / 4,
		.h = screenheight / 4
	};
	long long ticks = 0;
	long long frames = 0;
	long long over_frames = 0;
	struct newstate new_state;
	struct curstate cur_state;
	char keys[] = "";
	for (;;){
		frames++;
		ticks = SDL_GetTicks64();
		strcpy(keys, "");

		/* May want to put this in a separate subroutine. */
		while (SDL_PollEvent(&event)){
			switch (event.type){
			case SDL_QUIT:
				quit++;
				break;
			case SDL_MOUSEBUTTONDOWN:
				if (event.button.button == SDL_BUTTON_LEFT){
					click.x = event.button.x;
					click.y = event.button.y;
					click.unhandled++;
				}
				break;
			case SDL_KEYDOWN:
				if (event.key.keysym.sym < 0x7f && event.key.keysym.sym > 8)
					strncat(keys, (char *)&event.key.keysym.sym, 1);
			}
		}
		if (quit) break;

		/* Could this be put in a separate subroutine?  Yeah, but this is just a placeholder, so whatever. */
		if (click.unhandled){
			short x;
			short y;
			const short x4 = screenwidth/4;
			const short y4 = screenheight/4;

			if (click.x >= 0 && click.x < x4) x = 0;
			else if (click.x >= x4 && click.x < (x4 * 2)) x = 1;
			else if (click.x >= (x4 * 2) && click.x < (x4 * 3)) x = 2;
			else x = 3;

			if (click.y >= 0 && click.y < y4) y = 0;
			else if (click.y >= y4 && click.y < (y4 * 2)) y = 1;
			else if (click.y >= (y4 * 2) && click.y < (y4 * 3)) y = 2;
			else y = 3;

			squares[x][y] = !squares[x][y];
			click.unhandled = 0;
		}

		for (register short i = 0; i < gpcount; i++){
			cur_state.frame = frames;
			strcpy(cur_state.keys, keys);
			memcpy(cur_state.grid, squares, (sizeof squares) / (sizeof (_Bool)));
			new_state = programs[i](cur_state);
			memcpy(squares, new_state.grid, (sizeof squares) / (sizeof (_Bool)));
		}

		SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);
		SDL_RenderClear(renderer);

		SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
		for (int x = 0; x < 4; x++){
			for (int y = 0; y < 4; y++){
				if (squares[x][y] == 0){
					const short x4 = screenwidth/4;
					const short y4 = screenheight/4;
					rect.x = x4 * x;
					rect.y = y4 * y;
					SDL_RenderFillRect(renderer, &rect);
				}
			}
		}

		SDL_RenderPresent(renderer);

		if ((SDL_GetTicks64() - ticks) > 27){
			over_frames++;
			if (over_frames % 36 == 0){
				logmsg(lp_note, lc_env, "Slowdown detected.  Continued slowdown could result in a crash.");
			}
			if (over_frames > 360){
				crash(0x0E, "No extra information.");
			}
		} else if (over_frames > 0){
			over_frames--;
		}
	}

	logmsg(lp_info, lc_misc, "Exiting Gake…");

	for (register short i = 0; i < gpcount; i++){
		dlclose(tables[i]);
	}

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);

	SDL_Quit();

	halt_logging();
}
