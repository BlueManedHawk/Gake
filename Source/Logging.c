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

/* This file contains the logging system for Gake.  Priorities and categories are defined in `Logging.h`.  There is no `lp_critical` priority—the only way to log a critical error is by crashing the game.
 *
 * When reading this file, you are expected to have access to and generally understand the following documents:
 * 	· Latest draft of C2x:  http://www.open-std.org/JTC1/SC22/WG14/www/docs/n2596.pdf
 * 	· The Clang compiler user(?) manual:  https://clang.llvm.org/docs/UsersManual.html
 * 	· Wikipedia page on ANSI escape codes:  https://en.wikipedia.org/wiki/ANSI_escape_code
 * 	· The latest POSIX specification:  https://pubs.opengroup.org/onlinepubs/9699919799/mindex.html */

#define _POSIX_C_SOURCE 200809L

#include "Logging.h"
#include <stdarg.h>
#include <stdio.h>
#include <time.h>
#include <stdbool.h>
#include <stdlib.h>

/*static FILE * logfile = NULL;

void setup_logging(void){
	char * state = getenv("XDG_STATE_HOME");
	if (state == NULL){
		char * home = getenv("HOME");
		if (home != NULL){
			sprintf(state, "%s/.local/state", home);
		}
	}

	char logfile_location[256];
	char time_str[64];
	time_t the_time = time(NULL);
	strftime(time_str, 64, "%F_%T", localtime(&the_time));

	if (state != NULL){
		snprintf(logfile_location, 256, "%s/Gake/log_%s.log", state, time_str);
		logfile = fopen(logfile_location, "a");
		if (logfile == NULL){
			snprintf(logfile_location, 256, "/tmp/Gake/log_%s.log", time_str);
			logfile = fopen(logfile_location, "a");
			fprintf(stderr, "\e[1;33mNotice:  An appropriate location for a log file could not be found.  Please make sure that either \e[1;4m$HOME\e[1;33m or \e[1;4m$XDG_STATE_HOME\e[1;33m is set.");
		}
	} else {
		snprintf(logfile_location, 256, "/tmp/Gake/log_%s.log", time_str);
		logfile = fopen(logfile_location, "a");
		fprintf(stderr, "\e[1;33mNotice:  An appropriate location for a log file could not be found.  Please make sure that either \e[1;4m$HOME\e[1;33m or \e[1;4m$XDG_STATE_HOME\e[1;33m is set.");
	}
}

void halt_logging(void){
	fclose(logfile);
} */

/* Note that `logmsg` assumes that you've sanitized the string before you log it. */
[[gnu::format(printf, 3, 4)]] void logmsg(enum log_priority priority, enum log_category category, char * msg, ...){
	char final_message[1024], time_str[64], priority_str[32], category_str[32];
	switch (priority){
	/* See `Logging.h`—this makes debug messages only show in debug builds.*/
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wswitch"
	case -1:
		goto end;
	case 1:
		sprintf(priority_str, "\e[32mDEBUG\e[m");
		break;
#pragma clang diagnostic pop
	case lp_info:
		sprintf(priority_str, "\e[37mINFO\e[m");
		break;
	case lp_note:
		sprintf(priority_str, "\e[36mNOTICE\e[m");
		break;
	case lp_warn:
		sprintf(priority_str, "\e[33mWARNING\e[m");
		break;
	case lp_err:
		sprintf(priority_str, "\e[31mERROR\e[m");
		break;
	default:
		sprintf(priority_str, "\e[34m???????\e[m");
		break;
	}

	switch (category){
	case lc_misc:
		sprintf(category_str, "Miscellaneous");
		break;
	case lc_debug:
		sprintf(category_str, "Debugging systems");
		break;
	case lc_env:
		sprintf(category_str, "Host environment");
		break;
	default:
		sprintf(category_str, "Unknown category");
		break;
	}

	time_t the_time = time(NULL);
	if (strftime(time_str, 64, "%T", localtime(&the_time)) == 0){
		sprintf(time_str, "??:??:??");
	}

	char formatted_msg[512];
	va_list ap;
	va_start(ap, msg);
	vsnprintf(formatted_msg, 512, msg, ap);
	va_end(ap);
	snprintf(final_message, 1024, "[%s] %s (%s:)  %s\n", time_str, priority_str, category_str, formatted_msg);
	fprintf(stderr, "%s", final_message);
//	fprintf(logfile, "%s", final_message);
end:
	((void)0);
}
