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

/* This file is for things done during the setup of gake (except the `setup_logging()` function, which is from `Source/Logging.c`).  Currently, this is setting up the signal handler, and sometime after vN.1, this will include handling the arguments sent to Gake.
 *
 * When reading this file, you are expected to have access to and generally understand the following documents:
 * 	· Latest draft of C2x:  http://www.open-std.org/JTC1/SC22/WG14/www/docs/n2596.pdf
 * 	· The Clang compiler user(?) manual:  https://clang.llvm.org/docs/UsersManual.html
 * 	· The latest POSIX specification:  https://pubs.opengroup.org/onlinepubs/9699919799/mindex.html */

#include <signal.h>
#include "Crash.h"
#include <stddef.h> /* Heh.  Can't believe that I have to explicitly include this. */

void install_signals(void)
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

}
