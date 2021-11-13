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

#ifndef CRASH_H
#define CRASH_H

#include <signal.h>
#include <stdint.h>
#include <stdarg.h>

extern uint8_t crashno;
extern char crashstr[512];

extern void crash(uint8_t code, char * info, ...);
extern void handler(int signo, siginfo_t * info, void * context);

#endif
