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

/* This file defines the crash handler for Gake, `handler()`, and a subroutine to create arbitrary crashes, `crash()`.
 *
 * When reading this file, you are expected to have access to and generally understand the following documents:
 * 	· Latest draft of C2x:  http://www.open-std.org/JTC1/SC22/WG14/www/docs/n2596.pdf
 * 	· The Clang compiler user(?) manual:  https://clang.llvm.org/docs/UsersManual.html
 * 	· The latest POSIX specification:  https://pubs.opengroup.org/onlinepubs/9699919799/mindex.html */


#define _POSIX_C_SOURCE 200809L

#include <stdint.h>
#include <stdarg.h>
#include <signal.h>
#include <stdio.h>
#include "SDL.h"
#include <time.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdbool.h>

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
	/*0x0B*/"An invalid system call was made.",
	/*0x0C*/"The game's assets could not be verified.",
	/*0x0D*/"Your system does not have enough battery left.",
	/*0x0E*/"Your system is too slow.  The game has been crashed to prevent cheating."
};

[[gnu::format(printf, 2, 3)]] void crash(uint8_t code, char * info, ...)
{
	crashno = code;
	va_list ap;
	va_start(ap, info);
	vsnprintf(crashstr, sizeof crashstr, info, ap);
	va_end(ap);
	raise(SIGUSR1); /* We're using this signal to indicate a program-instantiated crash. Theorhetically, someone could just raise this manually, but this is the preferred way to do it. */
}

/* This isn't written the best, and could probably be reorganized to make a bit more sense. */
/* TODO:  This crash handler should have a nicer interface through SDL, instead of just using messageboxes. */
void handler(int signo, siginfo_t * info, [[maybe_unused]] void * context)
{
	bool perform_default = 0;

	char details[256] = "No other details.";

	/* There's gotta be a better way to write this (probably with a pair of arrays), but since this will only be called at most once during each execution, optimization is not a priority of mine. */
	switch (signo){
	case SIGABRT:
		perform_default = 1;
		crashno = 0x03;
		break;
	case SIGBUS:
		perform_default = 1;
		crashno = 0x04;
		switch (info->si_code){
		case BUS_ADRALN:
			snprintf(details, sizeof details, "The address %p is not a valid alignment.", info->si_addr);
			break;
		case BUS_ADRERR:
			snprintf(details, sizeof details, "The address %p does not exist.", info->si_addr);
			break;
		case BUS_OBJERR:
			snprintf(details, sizeof details, "The address %p is the source of this bus error.", info->si_addr);
			break;
		}
		break;
	case SIGFPE:
		perform_default = 1;
		crashno = 0x05;
		switch (info->si_code){
		case FPE_INTDIV:
			snprintf(details, sizeof details, "At %p, the program tried to divide an integer by zero.", info->si_addr);
			break;
		case FPE_INTOVF:
			snprintf(details, sizeof details, "At %p, the program overflowed an integer.", info->si_addr);
			break;
		case FPE_FLTDIV:
			snprintf(details, sizeof details, "At %p, the program tried to divide a float by zero.", info->si_addr);
			break;
		case FPE_FLTOVF:
			snprintf(details, sizeof details, "At %p, the program overflowed a float.", info->si_addr);
			break;
		case FPE_FLTUND:
			snprintf(details, sizeof details, "At %p, the program underflowed a float.", info->si_addr);
			break;
		case FPE_FLTRES:
			snprintf(details, sizeof details, "At %p, a float had an inexact result.", info->si_addr);
			break;
		case FPE_FLTINV:
			snprintf(details, sizeof details, "At %p, the program tried to perform an invalid floating-point operation.", info->si_addr);
			break;
		case FPE_FLTSUB:
			snprintf(details, sizeof details, "At %p, a float had an invalid subscript.", info->si_addr);
			break;
		}
		break;
	case SIGHUP:
		perform_default = 1;
		crashno = 0x06;
		break;
	case SIGILL:
		perform_default = 1;
		crashno = 0x07;
		switch (info->si_code){
		case ILL_ILLOPC:
			snprintf(details, sizeof details, "At %p, the program tried to use an illegal opcode.", info->si_addr);
			break;
		case ILL_ILLOPN:
			snprintf(details, sizeof details, "At %p, the program tried to perform an instruction with an illegal operand.", info->si_addr);
			break;
		case ILL_ILLADR:
			snprintf(details, sizeof details, "At %p, the program was in an illegal addressing mode.", info->si_addr);
			break;
		case ILL_ILLTRP:
			snprintf(details, sizeof details, "At %p, the program performed an illegal trap.", info->si_addr);
			break;
		case ILL_PRVOPC:
			snprintf(details, sizeof details, "At %p, the program tried to use a privleged opcode.", info->si_addr);
			break;
		case ILL_PRVREG:
			snprintf(details, sizeof details, "At %p, the program tried to use a privleged register.", info->si_addr);
			break;
		case ILL_COPROC:
			snprintf(details, sizeof details, "At %p, an error occurred in a coprocessor.", info->si_addr);
			break;
		case ILL_BADSTK: //bad stick!
			snprintf(details, sizeof details, "At %p, something went wrong with the stack.", info->si_addr);
			break;
		}
		break;
	case SIGSEGV: /* Note to self:  make this do something unique when this code is ported to use a fancy proper window instead of an SDL messagebox. */
		perform_default = 1;
		crashno = 0x09;
		switch (info->si_code){
		case SEGV_MAPERR:
			snprintf(details, sizeof details, "The address at %p is not mapped to anything.", info->si_addr);
			break;
		case SEGV_ACCERR:
			snprintf(details, sizeof details, "The address at %p is mapped to something innaccessible.", info->si_addr);
			break;
		}
		break;
	case SIGSYS:
		perform_default = 1;
		crashno = 0x0B;
		break;
	/* Note to future:  These next two should be handled specially when things are actually implemented. */
	case SIGTERM:
		perform_default = 1;
		crashno = 0x0A;
		goto nothing;
	case SIGQUIT:
		perform_default = 1;
		crashno = 0x08;
		[[fallthrough]];
	/* This could probably be used for something, but for now, it does nothing. */
	case SIGINT:
		goto nothing;
	case SIGUSR1:
		break;
	}

	char msg_box_msg[1024];
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

	time_t the_time = time(NULL);
	char time_str[64];
	if (strftime(time_str, sizeof time_str, "%T", localtime(&the_time)) == 0){
		sprintf(time_str, "??:??:??");
	}

	printf("[%s] \e[41mCRITICAL\e[m\e[31m:  %s\e[m", time_str, msg_box_msg);

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
		char share[64];
		char * shareptr = getenv("XDG_DATA_HOME");
		if (shareptr == NULL){
			char * home = getenv("HOME");
			if (home == NULL){
				printf("\e[33mSorry, but an appropriate place to log this crash could not be found because neither `$XDG_DATA_HOME` nor `$HOME` was set.\e[m\n");
				SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "No valid logging location", "Sorry, but an appropriate place to log this crash could not be found because neither `$XDG_DATA_HOME` nor `$HOME` was set.", NULL);
			} else {
				sprintf(share, "%s/.local/share", home);
			}
		} else {
			strcpy(share, shareptr);
		}
		char crash_report_name[128] = {};
		char time_str[64];
		time_t the_time = time(NULL);
		strftime(time_str, 64, "%F_%T", localtime(&the_time));
		/* Again, there's something to be said about optimization, but again, this is only ever going to be run once at most during the program lifetime. */
		snprintf(crash_report_name, 128, "%s/Gake/", share); // Temporarily use `crash_report_name` for the directory where the report should go.
		DIR * crash_report_dir = opendir(crash_report_name);
		if (crash_report_dir == NULL){
			if (errno == ENOENT){
				mkdir(crash_report_name, 0755);
			}
		}
		closedir(crash_report_dir);
		memset(crash_report_name, 0, 128);
		snprintf(crash_report_name, 128, "%s/Gake/Crash_Reports/", share);
		crash_report_dir = opendir(crash_report_name);
		if (crash_report_dir == NULL){
			if (errno == ENOENT){
				mkdir(crash_report_name, 0755);
			}
		}
		closedir(crash_report_dir);
		memset(crash_report_name, 0, 128);
		snprintf(crash_report_name, 128, "%s/Gake/Crash_Reports/%s.txt", share, time_str);
		FILE * crash_report = fopen(crash_report_name, "a");
		if (crash_report == NULL){
			char logerrormsg[256];
			snprintf(logerrormsg, 256, "The logfile at\n %s \ncould not be opened for writing.", crash_report_name);
			printf("\e[33m%s\e[m\n", logerrormsg);
			SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Unopenable logfile", logerrormsg, NULL);
		} else {
			fprintf(crash_report, "%s", msg_box_data.message);
			fclose(crash_report);
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
	((void)0);
}
