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

/* This file contains a subroutine run at program startup to verify the assets and check the battery level.
 *
 * When reading this file, you are expected to have access to and generally understand the following documents:
 * 	· Latest draft of C2x:  http://www.open-std.org/JTC1/SC22/WG14/www/docs/n2596.pdf
 * 	· The Clang compiler user(?) manual:  https://clang.llvm.org/docs/UsersManual.html
 * 	· The latest POSIX specification:  https://pubs.opengroup.org/onlinepubs/9699919799/mindex.html */


#define _POSIX_C_SOURCE 200809L
#define GCRYPT_NO_DEPRECATED

#include "SDL.h"
#include "Checks.h"
#include "Logging.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <gcrypt.h> /* TODO: I'm not happy about the usage of GNU stuff, especially a package as bloated as this one. */
#include <string.h>
#include <stdbool.h>

static const struct file_data files_list[2] = {
	{{0x8277519b0267bc80, 0x46b8fb577867402d, 0x9866eb94bb6592f7, 0x3d588676681a8e5a, 0xa1af99a4767c7378, 0xf758ade27bffe5c4, 0x2f961c59d1748173, 0xe6d39fd96d567188}, 443, "/usr/local/share/Gake/Assets/Textures.png"},
	{{0xca19a88b98f18c9d, 0x64071c594d5ff2a8, 0x998f3ed76494626a, 0x8eb5f88388eb05d9, 0x4135c3fc843350ea, 0xd58d2a2beac843fa, 0x20c3e97bfb71d39b, 0x9eb27807e2c5993e}, 1959, "/usr/local/share/Gake/Assets/Log_Splashes.txt"}
};

short run_checks(void)
{
	logmsg(lp_debug, lc_checks, "Verifying assets…");
	uint8_t buf[0xFFF] = {};
	FILE * file;
	long long checksum[8];
	size_t size;
	bool errors = 0;
	for (register unsigned i = 0; i < ((sizeof files_list) / (sizeof (struct file_data))); i++){
		logmsg(lp_debug, lc_checks, "Testing asset %s…", files_list[i].filename);
		if ((file = fopen(files_list[i].filename, "rb")) == NULL){
			logmsg(lp_err, lc_checks, "File %s does not exist.", files_list[i].filename);
			errors = 1;
		} else if ((size = fread(buf, sizeof (uint8_t), (sizeof buf) / (sizeof (uint8_t)), file)) != files_list[i].size){
			logmsg(lp_err, lc_checks, "Expected file %s to have size %zd, but got size %zd.", files_list[i].filename, files_list[i].size, size);
			errors = 1;
		} else {
			gcry_md_hash_buffer(GCRY_MD_SHA3_512, &checksum, &buf, sizeof buf);
			if (memcmp(checksum, files_list[i].checksum, sizeof checksum)){
				char realsum_str[129] = "";
				char badsum_str[129] = "";
				char sect[16];
				for (register size_t j = 0; j < ((sizeof checksum) / sizeof (long long)); j++){
					sprintf(sect, "%llx", checksum[j]);
					strcat(badsum_str, sect);
				}
				for (register size_t j = 0; j < ((sizeof files_list[i].checksum) / sizeof (long long)); j++){
					sprintf(sect, "%llx", files_list[i].checksum[j]);
					strcat(realsum_str, sect);
				}
				logmsg(lp_err, lc_checks, "Expected file %s to have checksum %s, but got checksum %s.", files_list[i].filename, realsum_str, badsum_str);
				errors = 1;
			}
		}
		fclose(file);
		memset(buf, 0, sizeof buf);
	}
	if (errors){
		logmsg(lp_err, lc_checks, "Assets could not be verified.  Crashing now…");
		return 1;
	}
	logmsg(lp_info, lc_checks, "All assets have been verified!");

	char * logsplash = NULL;
	int nlcount = 0;
	int character = 0;
	FILE * splashfile = fopen("/usr/local/share/Gake/Assets/Log_Splashes.txt", "r");
	if (splashfile == NULL){
fail:
		logmsg(lp_info, lc_checks, "\e[38;2;255;255;128mFailing to find a log splash…\e[m");
	} else {
		size_t len = 0;
		fseek(splashfile, 0, SEEK_SET);
		for (register int i = 0; (character = fgetc(splashfile)) != EOF; i++)
			(character == '\n') ? nlcount++ : ((void)0);

		FILE * random = fopen("/dev/urandom", "rb");
		long long randnum[1];
		fread(randnum, sizeof (long long), (sizeof randnum) / sizeof (long long), random);
		fclose(random);
		int line = llabs(randnum[0] % nlcount);

		fseek(splashfile, 0, SEEK_SET);
		for (register int i = 0; i < line; i++){
			getline(&logsplash, &len, splashfile);
		}

		if (logsplash == NULL)
			goto fail;

		char * nl;
		if ((nl = strchr(logsplash, '\n')) != NULL)
			*nl = ' ';

		logmsg(lp_info, lc_checks, "\e[38;2;255;255;128m%s\e[m", logsplash);
	}
	free(logsplash);
	fclose(splashfile);

	int secs, pct;
	SDL_PowerState power = SDL_GetPowerInfo(&secs, &pct);
	switch (power){
	case SDL_POWERSTATE_UNKNOWN:
	unknown:
		logmsg(lp_warn, lc_checks, "Battery state is indeterminable!  Warning user…");
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_WARNING, "Gake | Battery State Indeterminable", "Please check your battery before playing.", NULL);
		[[fallthrough]];
	case SDL_POWERSTATE_NO_BATTERY:
		return 0;
	case SDL_POWERSTATE_ON_BATTERY:
		if (secs == -1 && pct == -1){
			goto unknown;
		} else if ((secs < 900) || (pct < 15)){
			logmsg(lp_err, lc_checks, "User does not have enough battery left!");
			return 2;
		}
		[[fallthrough]];
	case SDL_POWERSTATE_CHARGING:
	case SDL_POWERSTATE_CHARGED:
		logmsg(lp_info, lc_checks, "Battery is fine.");
		return -1;
	}

}
