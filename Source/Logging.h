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

#ifndef LOGGING_H
#define LOGGING_H

#include <stdarg.h>

enum log_priority {
#ifdef GAKE_DEBUG
	lp_debug = 1,
#else
	lp_debug = -1,
#endif
	lp_info = 2,
	lp_note = 3,
	lp_warn = 4,
	lp_err = 5
	/* See `Logging.c` for why there's no `lp_critical`. */
};

enum log_category {
	lc_misc = 0,
	lc_debug = 1,
	lc_env = 2,
	lc_checks = 3,
	lc_api = 4,
	lc_apiprgm = 5,
	/* More categories will prove necessary.  If you add a new category here, be sure to update the list in `Source/Logging.c`, too. */
};

extern void setup_logging(void);
extern void halt_logging(void);
/* Keep in mind that this function is not sanitized—you'll need to do that yourself. */
extern void logmsg(enum log_priority priority, enum log_category category, char * msg, ...);
extern void vlogmsg(enum log_priority priority, enum log_category category, char * msg, va_list arg);

#endif/*ndef LOGGING_H*/
