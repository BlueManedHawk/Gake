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

/* This is the crash handler for Gake.
 *
 * When reading this file, you are expected to have access to and generally understand the following documents:
 * 	· Latest draft of C2x:  http://www.open-std.org/JTC1/SC22/WG14/www/docs/n2596.pdf
 * 	· The Clang compiler user(?) manual:  https://clang.llvm.org/docs/UsersManual.html
 * 	· Wikipedia page on ANSI escape codes:  https://en.wikipedia.org/wiki/ANSI_escape_code
 * 	· The latest POSIX specification:  https://pubs.opengroup.org/onlinepubs/9699919799/mindex.html */

#include "Crash.h"
#include <stdnoreturn.h>
#include "SDL.h"
#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>

static const char arr_msgs[4][128] = {
	/*0*/ "Crash with successful exit.  This shouldn't happen.",
	/*1*/ "Technical failure.  This should not be handled by the crash handler.",
	/*2*/ "User has attempted to use GNU-style options when starting program."
} ;

noreturn void crash(uint8_t code, [[maybe_unused]] char * info_fmt, ...){

	char msg[128];
	strcpy(msg, arr_msgs[code]);
	va_list vargs;
	va_start(vargs, info_fmt);
	char info[256];
	vsnprintf(info, 256, info_fmt, vargs);
	va_end(vargs);

	time_t timer = time(NULL);
	struct tm * time = localtime(&timer);
	char time_str[128];
	strftime(time_str, 64, "%X", time);
	printf("[%s] \e[41;1mCRITICAL:\e[m\e[31;1m Gake has crashed!  Error code: %d, which means:\n\n%s\n\nAdditional info:\n\n%s\n\nQuitting now…\n\e[m", time_str, code, msg, info);

	char msg_box_str[512];
	sprintf(msg_box_str,
	"Gake has crashed!\n"
	"\n"
	"Exit code:  %d\n"
	"which means:  %s\n"
	"\n"
	"Additional information:  %s\n"
	"\n"
	"Crash occurred at %s.\n"
	"\n"
	"Would you like to save this crash report?", code, msg, info, time_str);

	SDL_MessageBoxButtonData buttons[2] = {
		{SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT, 0, "Nah, just quit"},
		{SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, 1, "Save crash report"}
	};
	SDL_MessageBoxData msg_data = {
		SDL_MESSAGEBOX_ERROR,
		NULL,
		"Gake Crash Handler",
		msg_box_str,
		2,
		buttons,
		NULL /* Might want to use a different one at some point. */
	};
	int button = 0;
	SDL_ShowMessageBox(&msg_data, &button);
	if (button == 1){
		printf("\e[33mCrash report saving has not yet been implemented.  Sorry!\e[m\n");
	}
	exit(code);
}
