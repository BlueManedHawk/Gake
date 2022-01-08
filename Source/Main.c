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

int main(int argc, char ** argv)
{

	struct sigaction act = {
		.sa_sigaction = handler,
		.sa_mask = { 0 },
		.sa_flags = SA_SIGINFO
	};
	/* If you ever need to update these, be certain that the terminating zeros remain. */
	int sigs_to_handle[16] = {SIGABRT, SIGBUS, SIGFPE, SIGHUP, SIGILL, SIGINT, SIGQUIT, SIGSEGV, SIGTERM, SIGUSR1, SIGSYS, 0};
	int sigs_to_ign[8] = {SIGALRM, SIGPIPE, SIGUSR2, SIGVTALRM, 0};
	for (register int i = 0; sigs_to_handle[i] != 0; i++){
		sigaction(sigs_to_handle[i], &act, NULL);
	}
	struct sigaction ign = {
		.sa_handler = SIG_IGN
	};
	for (register int i = 0; sigs_to_ign[i] != 0; i++){
		sigaction(sigs_to_ign[i], &ign, NULL);
	}

	for (signed char opts = 0; opts != -1; opts = getopt(argc, argv, "?hv-")){
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
	case 2:
	case 3:
		crash(0xC, "See the game log for details.");
		break;
	case 4:
		crash(0xD, "No other details.");
	}

	halt_logging();
}
