/* LICENSE
 *
 * Copyright © 2021 Blue-Maned_Hawk.  All rights reserved.
 *
 * This software should have come with a file called LICENSE.  In case of any difference between this comment and that file, that file is the authority.  (If you did not recieve that file, it's a violation of the license.  Please report it to me.)
 *
 * This project is copylefted.  You may freely use, distribute, and modify this software, to the extent permitted by law, so long as you do not attempt to claim such activities are condoned by the author, you distribute the license file with any distributions of this software, you release any modifications under a similar license, and you do not attempt to claim that modified software is the original software.
 *
 * This license does not apply to software created with the API of this software (thought it does apply to the API itself), and it does not apply to any rule files, all of which must be placed in the public domain.
 *
 * This software dynamically links to SDL2, which is under the zlib license, available at https://libsdl.org/license.php.
 *
 * This license does not apply to trademarks or patents.
 *
 * THIS PRODUCT COMES WITH ABSOLUTELY NO WARRANTY, IMPLIED OR EXPLICIT, TO THE EXTENT PERMITTED BY LAW.  THE AUTHOR DISCLAIMS ANY LIABILITY FOR ANY DAMAGES OF ANY KIND CAUSED BY THIS PRODUCT, TO THE EXTENT PERMITTED BY LAW.*/

/* This is the main file for Gake.  It contains the `main()` function.
 *
 * When reading this file, you are expected to have access to and generally understand the following documents:
 * 	· Latest draft of C2x:  http://www.open-std.org/JTC1/SC22/WG14/www/docs/n2596.pdf
 * 	· The Clang compiler user(?) manual:  https://clang.llvm.org/docs/UsersManual.html
 * 	· Wikipedia page on ANSI escape codes:  https://en.wikipedia.org/wiki/ANSI_escape_code
 * 	· The latest POSIX specification:  https://pubs.opengroup.org/onlinepubs/9699919799/mindex.html */

#ifdef __cplusplus
#	error "\e[5m\e[1m\e[31mTHIS IS NOT A C++ PROGRAM."
#endif

#if __clang_major__ != 13
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

	for (signed char opts = 0; opts != -1; opts = getopt(argc, argv, "?hv-il:")){
		switch (opts){
		case 0:
			break;
		case '-':
			printf(
			"\e[31m\e[1mGake does not and never will support GNU-style options.\e[m\n"
			"\n"
			"(If you were trying to use \e[4m--help\e[m or \e[4m--version\e[m, please use \e[4m-h\e[m or \e[4m-v\e[m.)\n");
			crash(2, "No extra info.");
		case '?':
		case 'h':
			printf(
			"Gake is an open-source reimplementation of Google Snake, with extensions.\n"
			"\n"
			"Currently, the only options supported are \e[4m-v\e[m, which states the version, and \e[4m-h\e[m and \e[4m-?\e[m, both of which give this help blurb.  If you specify no options, the game will start normally.\n"
			"\n"
			"For more information, please see the manpage (available with \e[1mman gake\e[m, if installed).\n"
			"\n"
			"\e[1mThis program does not and never will support GNU-style options.\e[m\n");
			return 1; /* Counted as a failure for consistency with other software and not breaking things like `make`. */
		case 'v':
			printf("This is Gake vN.0, semantic version 0.0.0.\n");
			return 1; /* See above comment. */
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

	SDL_Init(SDL_INIT_VIDEO);
	SDL_Window * window = SDL_CreateWindow("Gake", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, 0);
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
		.w = 640 / 4,
		.h = 480 / 4
	};
	long long ticks = 0;
	int over_frames = 0;

	for (;;){
		ticks = SDL_GetTicks64();
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
			}
		}
		if (quit) break;

		/* Could this be put in a separate subroutine?  Yeah, but this is just a placeholder, so whatever. */
		if (click.unhandled){
			short x;
			short y;
			const short x4 = 640/4;
			const short y4 = 480/4;

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

		SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);
		SDL_RenderClear(renderer);

		SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
		for (int x = 0; x < 4; x++){
			for (int y = 0; y < 4; y++){
				if (squares[x][y] == 0){
					const short x4 = 640/4;
					const short y4 = 480/4;
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

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);

	SDL_Quit();

	halt_logging();
}
