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
 * When reading this file, you should probably have access to the following documents:
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

#include <stdio.h>
#include <unistd.h>

int main(int argc, char ** argv) {
	for (signed char opts = 0; opts != -1; opts = getopt(argc, argv, "?hv-")){
		switch (opts){
		case 0:
			break;
		case '-':
			printf(
			"\e[31m\e[1mGake does not and never will support GNU-style options.\e[m\n"
			"\n"
			"(If you were trying to use \e[4m--help\e[m or \e[4m--version\e[m, please use \e[4m-h\e[m or \e[4m-v\e[m.)\n");
			return 2; /* Should be `crash(2, "");`, once the crash handler gets implemented. */
		case '?':
		case 'h':
			printf(
			"Gake is an open-source reimplementation of Google Snake, with extensions.\n"
			"\n"
			"Currently, the only options supported are \e[4m-v\e[m, which states the version, and \e[4m-h\e[m and \e[4m-?\e[m, both of which give this help blurb.\n"
			"\n"
			"\e[1mThis program does not and never will support GNU-style options.\e[m\n");
			return 1; /* Counted as a failure for consistency with other software and not breaking things like `make`. */
		case 'v':
			printf("This is Gake vN.0, semantic version 0.0.0.\n");
		}
	}
}
