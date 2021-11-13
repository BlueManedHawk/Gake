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

/* This is the crash handler for Gake.  It defines the signal handler, `handler()`, and an easy method to create arbitrary crashes, `crash()`.  It also has some other stuff.
 *
 * When reading this file, you are expected to have access to and generally understand the following documents:
 * 	· Latest draft of C2x:  http://www.open-std.org/JTC1/SC22/WG14/www/docs/n2596.pdf
 * 	· The Clang compiler user(?) manual:  https://clang.llvm.org/docs/UsersManual.html
 * 	· Wikipedia page on ANSI escape codes:  https://en.wikipedia.org/wiki/ANSI_escape_code
 * 	· The latest POSIX specification:  https://pubs.opengroup.org/onlinepubs/9699919799/mindex.html */

#include <stdint.h>
#include <stdarg.h>
#include <signal.h>
#include <stdio.h>
#include "SDL.h"
#include <time.h>

uint8_t crashno;
char crashstr[512]; /* Should be enough for anything, right? */

static const char crash_msgs[16][128] = {
	/*0x00*/"Reserved for normal game exit.  If you are seeing this message, please report an issue.",
	/*0x01*/"Reserved for compatibility errors.  If you are seeing this message, please report an issue.",
	/*0x02*/"User has attempted to use GNU-style options.",
	/*0x03*/"Process was aborted.",
	/*0x04*/"Program attempted to access nonaddressable memory.",
	/*0x05*/"A floating-point exception has occurred.",
	/*0x06*/"Controlling terminal has been closed.",
	/*0x07*/"Program attempted a nonexistent CPU instruction.",
	/*0x08*/"External circumstances requested a graceful quit.",
	/*0x09*/"A segmentation fault occurred.",
	/*0x0A*/"Termination request sent to program.",
	/*0x0B*/"An invalid system call was made."};

[[gnu::format(printf, 2, 3)]] void crash (uint8_t code, char * info, ...)
{
	crashno = code;
	va_list ap;
	va_start(ap, info);
	vsnprintf(crashstr, sizeof crashstr, info, ap);
	va_end(ap);
	raise(SIGUSR1); /* We're using this signal to indicate a program-instantiated crash. */
}

/* This isn't written the best, and could probably be reorganized to make a bit more sense. */
void handler(int signo, [[maybe_unused]] siginfo_t * info, [[maybe_unused]] void * context)
{
	_Bool perform_default = -0;

	char details[512] = "No details given."; /* I intend to eventually have this get filled with information from `info`, but that's not strictly necessary, so for now it does nothing. */

	switch (signo){

	case SIGABRT:
		perform_default = 1;
		crashno = 0x03;
		break;
	case SIGBUS:
		perform_default = 1;
		crashno = 0x04;
		break;
	case SIGFPE:
		perform_default = 1;
		crashno = 0x05;
		break;
	case SIGHUP:
		perform_default = 1;
		crashno = 0x06;
		break;
	case SIGILL:
		perform_default = 1;
		crashno = 0x07;
		break;
	case SIGSEGV: /* Note to self:  make this do something unique. */
		perform_default = 1;
		crashno = 0x09;
		break;
	case SIGSYS:
		perform_default = 1;
		crashno = 0x0B;
		break;
	/* Note to future:  These next two should be handled specially when things are actually implemented. */
	case SIGTERM:
		perform_default = 1;
		crashno = 0x0A;
	case SIGQUIT:
		perform_default = 1;
		crashno = 0x0B;
	/* This could probably be used for something, but for now, it does nothing. */
	case SIGINT:
		goto nothing;
	case SIGUSR1:
		break;
	}

	char msg_box_msg[512];
	sprintf(msg_box_msg,
			"A critical error has occured, and Gake has crashed!\n"
			"\n"
			"Gake exited with a code of %d, which means:\n"
			"\t%s\n"
			"\n"
			"Details on this error:\n"
			"\t%s"
			"\n"
			"Other information for this error:\n"
			"\n"
			"\t%s\n"
			"\n"
			"Would you like to save this crash report?\n",
			crashno, crash_msgs[crashno], details, crashstr);
	printf("[time] \e[41mCRITICAL\e[m\e[31m:  %s", msg_box_msg);

	SDL_MessageBoxButtonData buttons[2] = {
		{ SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT, 0, "Just quit" },
		{ SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, 1, "Save report" }
	};
	SDL_MessageBoxData msg_box_data = {
		SDL_MESSAGEBOX_ERROR,
		NULL,
		"Gake Crash Handler",
		msg_box_msg,
		2,
		buttons,
		NULL
	};
	int button_id = 0;
	SDL_ShowMessageBox(&msg_box_data, &button_id);

	if (button_id){
		char * share = getenv("XDG_DATA_HOME");
		if (share == NULL){
			char * home = getenv("HOME");
			if (home == NULL){
				printf("\e[33mSorry, but an appropriate place to log this crash could not be found because neither `$XDG_DATA_HOME` nor `$HOME` was set.\e[m\n");
			} else {
				sprintf(share, "%s/.local/share", home);
			}
		}
		char logfilename[64];
		/* This is horrible. */
		time_t the_time = time(NULL);
		struct tm * time_struct = gmtime(&the_time);
		char time_str[64];
		strftime(time_str, 64, "%F%T", time_struct);
		sprintf(logfilename, "%s/Gake/Crash_reports/%ld.txt", share, the_time);
		FILE * logfile = fopen(logfilename, "w");
		if (logfile == NULL){
			printf("\e[33mThe logfile at %s could not be opened for writing.\e[m\n", logfilename);
		} else {
			fprintf(logfile, "%s", msg_box_data.message);
		}
	}

	if (perform_default){
		struct sigaction ign = {
			.sa_handler = SIG_IGN
		};
		sigaction(signo, &ign, NULL);
		raise(signo);
	}

	exit(crashno);
nothing:
	printf("null\n");
}
