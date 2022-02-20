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

/* This defines the logging subroutines for Gake, along with a couple subroutines to start and stop it.  The enums for priorities and categories are in `Logging.h`.  There is no critical priority—the only way to log a message with such a priority is by crashing the game (see `Crash.c`).
 *
 * When reading this file, you are expected to have access to and generally understand the following documents:
 * 	· Latest draft of C2x:  http://www.open-std.org/JTC1/SC22/WG14/www/docs/n2596.pdf
 * 	· The Clang compiler user(?) manual:  https://clang.llvm.org/docs/UsersManual.html
 * 	· The latest POSIX specification:  https://pubs.opengroup.org/onlinepubs/9699919799/mindex.html */


#define _POSIX_C_SOURCE 200809L

#include "Logging.h"
#include <stdarg.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include "zlib.h" /* zlib's documentation is available at https://zlib.net/manual.html, but this program only uses the GZip file stuffs. */
#include <errno.h>
#include <sys/stat.h>
#include <dirent.h>

static gzFile logfile = NULL;
static const char categories[8][64] = {
	"Miscellaneous",
	"Debugging Systems",
	"Environment",
	"Runtime Checks",
	"API",
	"API-Using Programs"
};

void setup_logging(void)
{
	char logfilename[128];
	char time_str[64];
	time_t the_time = time(NULL);
	if (strftime(time_str, sizeof time_str, "%F_%T", localtime(&the_time)) == 0)
		snprintf(time_str, sizeof time_str, "UNKNOWN_TIME_%d", rand());
	char state[64];
	char * stateptr = getenv("XDG_STATE_HOME");
	if (stateptr == NULL){
		char * home = getenv("HOME");
		if (home != NULL){
			sprintf(state, "%s/.local/state", home);
			DIR * logdir = opendir(state);
			if (logdir == NULL){
				if (errno == ENOENT){
					mkdir(state, 0755);
				}
			}
			closedir(logdir);
		} else {
			sprintf(state, "/tmp");
			printf("\e[31mAn appropriate place for a logfile could not be found as neither $XDG_STATE_HOME nor $HOME was set.  This log will be saved in `/tmp/`.\n\e[m");
		}
	} else {
		strcpy(state, stateptr);
	}
	snprintf(logfilename, sizeof logfilename, "%s/Gake/", state); // Temporarily use `logfilename` for the directory where the logfile should go.
	DIR * logdir = opendir(logfilename);
	if (logdir == NULL){
		if (errno == ENOENT){
			mkdir(state, 0755);
		}
		strerror(errno);
	}
	closedir(logdir);
	memset(logfilename, 0, sizeof logfilename);
	snprintf(logfilename, sizeof logfilename, "%s/Gake/%s.txt.gz", state, time_str);
	logfile = gzopen(logfilename, "wb9");
}

void halt_logging(void)
{
	gzclose(logfile);
}

/* Note that `logmsg` assumes that you've sanitized the string before you log it. */
[[gnu::format(printf, 3, 4)]] void logmsg(enum log_priority priority, enum log_category category, char * msg, ...)
{
	va_list ap;
	va_start(ap, msg);
	vlogmsg(priority, category, msg, ap);
	va_end(ap);
}

void vlogmsg(enum log_priority priority, enum log_category category, char * msg, va_list arg)
{
	char final_message[1024], time_str[64], priority_str[32], category_str[32];
	switch (priority){
	/* See `Logging.h`—this makes debug messages only show in debug builds. This is kinda a silly way to do it, but oh well. */
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

	sprintf(category_str, "%s", categories[category]);

	time_t the_time = time(NULL);
	if (strftime(time_str, 64, "%T", localtime(&the_time)) == 0){
		sprintf(time_str, "??:??:??");
	}

	char formatted_msg[512];
	vsnprintf(formatted_msg, 512, msg, arg);
	snprintf(final_message, sizeof final_message, "[%s] %s (%s:)  %s\n", time_str, priority_str, category_str, formatted_msg);
	fprintf(stderr, "%s", final_message);
	if (logfile != NULL){ // Might not want to have this be checked every time this function is called, but I don't think there's a way around it.
		char * found_escape;
		/* Wtf‽ */
		while (NULL != (found_escape = strchr(final_message, (int)'\e'))){
			short j = 0;
			do {
				j++;
			} while (*(found_escape + j) != 'm');
			for (int k = 0; k <= j; k++)
				*(found_escape + k) = ' ';
		}
		gzprintf(logfile, "%s", final_message);
		/* zlib's manual states that `gzflush()` should only be called "when strictly necessary", stating that it will degrade performance if called too often.  I'm not sure whether it's best to use it like this. */
		gzflush(logfile, Z_NO_FLUSH);
	}
end:
	((void)0);
}
