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

/* This file is for functions for rendering the GUI of the game.
 *
 * When reading this file, you are expected to have access to and generally understand the following documents:
 * 	· Latest draft of C2x:  http://www.open-std.org/JTC1/SC22/WG14/www/docs/n2596.pdf
 * 	· The Clang compiler user(?) manual:  https://clang.llvm.org/docs/UsersManual.html
 * 	· The latest POSIX specification:  https://pubs.opengroup.org/onlinepubs/9699919799/mindex.html */

#include "SDL.h"
#include <stdbool.h>
#include "State.h"

static const short winheight = 480;
static const short winwidth = 640;

#define MKRECT_XYWH(name, xfract, yfract, wfract, hfract)\
	static SDL_Rect name = {\
		.x = winwidth * xfract,\
		.y = winheight * yfract,\
		.w = winwidth * wfract,\
		.h = winheight * hfract\
	}

/* REALLY wish that the C Preprocessor had loops. */
MKRECT_XYWH(gamebutton, 1/8, 1/6, 3/8, 2/3);
MKRECT_XYWH(gamegraphic, 3/16, 1/3, 1/4, 1/3);
MKRECT_XYWH(gameoffset, 1/8, 3/24, 3/8, 2/3);
MKRECT_XYWH(gamegrophset, 3/16, 7/24, 1/4, 1/3);
MKRECT_XYWH(prgmbutton, 5/8, 1/6, 1/4, 1/4 /* No, that's correct. */);
MKRECT_XYWH(prgmgraphic, 21/32 /*ew*/, 1/6, 3/16, 1/4 /* Also correct. */);
MKRECT_XYWH(prgmoffset, 5/8, 3/24, 1/4, 1/4);
MKRECT_XYWH(prgmgrophset, 21/32, 3/24, 3/16, 1/4);
MKRECT_XYWH(quitbutton, 5/8, 7/12, 1/4, 1/4);
MKRECT_XYWH(quitgraphic, 21/32, 7/12, 3/16, 1/4);
MKRECT_XYWH(quitoffset, 5/8, 13/24, 1/4, 1/4);
MKRECT_XYWH(quitgrophset, 21/32, 13/24, 3/16, 1/4);

static enum state selected = menu;
static enum state hover = menu;

enum state render_menu(struct mouse the_mouse, SDL_Keycode key, SDL_Renderer * renderer, SDL_Surface ** assets)
{
	SDL_Point mousepos = {
		.x = the_mouse.x,
		.y = the_mouse.y
	};
	bool click = the_mouse.mask & SDL_BUTTON_LMASK;
	SDL_Surface * surface = SDL_CreateRGBSurfaceWithFormat(0, winwidth, winheight, 32, SDL_PIXELFORMAT_RGBA32);
	int32_t light_blue_mask = SDL_MapRGBA(surface->format, 0x7f, 0x7f, 0xff, 0xff);
	int32_t blue_mask = SDL_MapRGBA(surface->format, 0x00, 0x00, 0xff, 0xff);
	int32_t dark_blue_mask = SDL_MapRGBA(surface->format, 0x00, 0x00, 0x7f, 0xff);
	int32_t light_red_mask = SDL_MapRGBA(surface->format, 0xff, 0x7f, 0x7f, 0xff);
	int32_t red_mask = SDL_MapRGBA(surface->format, 0xff, 0x00, 0x00, 0xff);
	int32_t dark_red_mask = SDL_MapRGBA(surface->format, 0x7f, 0x00, 0x00, 0xff);

	if (key == SDLK_INSERT || key == SDLK_RETURN || key == SDLK_DELETE || key == SDLK_ESCAPE)
		click++;

	if (selected != menu && !click){
		SDL_FreeSurface(surface);
		return selected;
	}

#define CHECK_MOUSE_FOR(rect)\
	if (SDL_PointInRect(&mousepos, &( rect ## button ))){\
		hover = rect;\
		selected = click ? rect : menu;}

	/* I don't think this is how the preprocessor is meant to be used. */
	CHECK_MOUSE_FOR(game)
	else CHECK_MOUSE_FOR(prgm)
	else CHECK_MOUSE_FOR(quit)
	else {
		hover = menu;
		selected = menu;
	}

	if (key == SDLK_INSERT || key == SDLK_RETURN)
		selected = game;
	else if (key == SDLK_DELETE || key == SDLK_ESCAPE)
		selected = quit;

#define RENDER(the_button, associated_asset, color)\
	if (selected == the_button){\
		SDL_FillRect(surface, &( the_button ## button ), light_ ## color ## _mask);\
		SDL_BlitScaled(associated_asset, NULL, surface, &( the_button ## graphic ));\
	} else if (hover == the_button){\
		SDL_FillRect(surface, &( the_button ## button ), color ## _mask);\
		SDL_FillRect(surface, &( the_button ## offset ), light_ ## color ## _mask);\
		SDL_BlitScaled(associated_asset, NULL, surface, &( the_button ## grophset ));\
	} else {\
		SDL_FillRect(surface, &( the_button ## button ), dark_ ## color ## _mask);\
		SDL_FillRect(surface, &( the_button ## offset ), color ## _mask);\
		SDL_BlitScaled(associated_asset, NULL, surface, &(the_button ## grophset ));\
	}

	RENDER(game, assets[0], blue);
	RENDER(prgm, assets[1], blue);
	RENDER(quit, assets[2], red);

	SDL_Texture * texture = SDL_CreateTextureFromSurface(renderer, surface);\
	SDL_RenderCopy(renderer, texture, NULL, NULL);

	SDL_DestroyTexture(texture);
	SDL_FreeSurface(surface);

	return menu;
}

enum state render_game(long long frames [[maybe_unused]], SDL_Keycode key [[maybe_unused]], struct mouse the_mouse [[maybe_unused]], SDL_Renderer * renderer [[maybe_unused]], SDL_Surface ** assets [[maybe_unused]]){
	return menu;
}

enum state render_prgm(struct mouse the_mouse [[maybe_unused]], SDL_Keycode key [[maybe_unused]], SDL_Renderer * renderer [[maybe_unused]]){
	return menu;
}
