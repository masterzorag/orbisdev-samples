/*
 * OpenTyrian: A modern cross-platform port of Tyrian
 * Copyright (C) 2007-2009  The OpenTyrian Development Team
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */
#include "keyboard.h"
#include "opentyr.h"
#include "palette.h"
#include "video.h"
#include "video_scale.h"

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char *scaling_mode_names[ScalingMode_MAX] = {
	"Center",
	"Integer",
	"Fit 8:5",
	"Fit 4:3",
};

int fullscreen_display;
ScalingMode scaling_mode = SCALE_INTEGER;
static SDL_Rect last_output_rect = { 0, 0, vga_width, vga_height };

SDL_Surface *VGAScreen, *VGAScreenSeg;
SDL_Surface *VGAScreen2;
SDL_Surface *game_screen;

#if defined USE_SDL2
SDL_Window *main_window = NULL;
#else
SDL_Surface *main_window_surface;  // the surface we sample2D to show on GLES2
#endif

// uses texture
static SDL_Renderer *main_window_renderer   = NULL;
SDL_PixelFormat     *main_window_tex_format = NULL;
static SDL_Texture  *main_window_texture    = NULL;

static ScalerFunction scaler_function;

static void init_renderer( void );
static void deinit_renderer( void );
static void init_texture( void );
static void deinit_texture( void );

static int window_get_display_index( void );
static void window_center_in_display( int display_index );
static void calc_dst_render_rect( SDL_Surface *src_surface, SDL_Rect *dst_rect );
static void scale_and_flip( SDL_Surface * );

void init_video( void )
{
#if defined USE_SDL2
	if (SDL_WasInit(SDL_INIT_VIDEO)) return;

	if (SDL_InitSubSystem(SDL_INIT_VIDEO) == -1)
	{
		fprintf(stderr, "error: failed to initialize SDL video: %s\n", SDL_GetError());
		exit(1);
	}
#endif

	// Create the software surfaces that the game renders to. These are all 320x200x8 regardless
	// of the window size or monitor resolution.
	VGAScreen = VGAScreenSeg = SDL_CreateRGBSurface(0, vga_width, vga_height, 8, 0, 0, 0, 0);
//printf("%x\n", SDL_ISPIXELFORMAT_INDEXED(VGAScreen->format->format));

// even if I clear palette it draws?
VGAScreen->format->palette = NULL;

	VGAScreen2  = SDL_CreateRGBSurface(0, vga_width, vga_height, 8, 0, 0, 0, 0);
	game_screen = SDL_CreateRGBSurface(0, vga_width, vga_height, 8, 0, 0, 0, 0);

	// create our destination surface we show on GLES2
	//main_window_surface = SDL_CreateRGBSurface(0, vga_width *2, vga_height *2, 16, 0, 0, 0, 0);
//printf("%x\n", SDL_ISPIXELFORMAT_INDEXED(main_window_surface->format->format));

#if defined USE_SDL2
	// The game code writes to surface->pixels directly without locking, so make sure that we
	// indeed created software surfaces that support this.
	assert(!SDL_MUSTLOCK(VGAScreen));
	assert(!SDL_MUSTLOCK(VGAScreen2));
	assert(!SDL_MUSTLOCK(game_screen));

	JE_clr256(VGAScreen);
	// Create the window with a temporary initial size, hidden until we set up the
	// scaler and find the true window size
	main_window = SDL_CreateWindow("OpenTyrian",
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		vga_width, vga_height, SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN);

	if (main_window == NULL)
	{
		fprintf(stderr, "error: failed to create window: %s\n", SDL_GetError());
		exit(EXIT_FAILURE);
	}

	reinit_fullscreen(fullscreen_display);
	init_renderer();

	init_texture();
#endif

	init_scaler(scaler);

#if defined USE_SDL2
	SDL_ShowWindow(main_window);
#endif

	input_grab(input_grab_enabled);
}

#if defined USE_SDL2
void deinit_video( void )
{
	deinit_texture();
	deinit_renderer();

	SDL_DestroyWindow(main_window);

	SDL_FreeSurface(VGAScreenSeg);
	SDL_FreeSurface(VGAScreen2);
	SDL_FreeSurface(game_screen);

	SDL_QuitSubSystem(SDL_INIT_VIDEO);
}

static void init_renderer()
{
	main_window_renderer = SDL_CreateRenderer(main_window, -1, 0);

	if (main_window_renderer == NULL)
	{
		fprintf(stderr, "error: failed to create renderer: %s\n", SDL_GetError());
		exit(EXIT_FAILURE);
	}
}

static void deinit_renderer( void )
{
	if (main_window_renderer != NULL)
	{
		SDL_DestroyRenderer(main_window_renderer);
		main_window_renderer = NULL;
	}
}
#endif

static void init_texture( void )
{
	// craft an SDL_PIXELFORMAT_RGB565

	int bpp = 16; // TODOSDL2
	uint32_t format = bpp == 32 ? SDL_PIXELFORMAT_RGB888 : SDL_PIXELFORMAT_RGB565;
	int scaler_w = scalers[scaler].width;
	int scaler_h = scalers[scaler].height;

	main_window_tex_format = SDL_AllocFormat(format);

	printf("format:%u, main_window_tex_format:%u, scaler_w:%d, scaler_w:%d\n", 
		format, main_window_tex_format, scaler_w, scaler_h);

	main_window_texture = SDL_CreateTexture(main_window_renderer, format, SDL_TEXTUREACCESS_STREAMING, scaler_w, scaler_h);

	if (main_window_texture == NULL)
	{
		fprintf(ERROR, "error: failed to create scaler texture %dx%dx%8x: %s\n", scaler_w, scaler_h, format, SDL_GetError());
		exit(EXIT_FAILURE);
	}
}

static void deinit_texture( void )
{
#if defined USE_SDL2
	if (main_window_texture != NULL)
	{
		SDL_DestroyTexture(main_window_texture);
		main_window_texture = NULL;
	}
#endif
	if (main_window_tex_format != NULL)
	{
		SDL_FreeFormat(main_window_tex_format);
		main_window_tex_format = NULL;
	}
}

#if defined USE_SDL2
static int window_get_display_index( void )
{
	return SDL_GetWindowDisplayIndex(main_window);
}

static void window_center_in_display( int display_index )
{
	int win_w, win_h;
	SDL_GetWindowSize(main_window, &win_w, &win_h);

	SDL_Rect bounds;
	SDL_GetDisplayBounds(display_index, &bounds);

	SDL_SetWindowPosition(main_window, bounds.x + (bounds.w - win_w) / 2, bounds.y + (bounds.h - win_h) / 2);
}

void reinit_fullscreen( int new_display )
{
	fullscreen_display = new_display;

	if (fullscreen_display >= SDL_GetNumVideoDisplays())
	{
		fullscreen_display = 0;
	}

	SDL_SetWindowFullscreen(main_window, SDL_FALSE);
	SDL_SetWindowSize(main_window, scalers[scaler].width, scalers[scaler].height);

	if (fullscreen_display == -1)
	{
		window_center_in_display(window_get_display_index());
	}
	else
	{
		window_center_in_display(fullscreen_display);

		if (SDL_SetWindowFullscreen(main_window, SDL_WINDOW_FULLSCREEN_DESKTOP) != 0)
		{
			reinit_fullscreen(-1);
			return;
		}
	}
}

void video_on_win_resize()
{
	int w, h;
	int scaler_w, scaler_h;

	// Tell video to reinit if the window was manually resized by the user.
	// Also enforce a minimum size on the window.

	SDL_GetWindowSize(main_window, &w, &h);
	scaler_w = scalers[scaler].width;
	scaler_h = scalers[scaler].height;

	if (w < scaler_w || h < scaler_h)
	{
		w = w < scaler_w ? scaler_w : w;
		h = h < scaler_h ? scaler_h : h;

		SDL_SetWindowSize(main_window, w, h);
	}
}

void toggle_fullscreen( void )
{
	if (fullscreen_display != -1) {
		reinit_fullscreen(-1);
	} else {
		reinit_fullscreen(SDL_GetWindowDisplayIndex(main_window));
	}
}
#endif

bool init_scaler( unsigned int new_scaler )
{
	int   w = scalers[new_scaler].width,
	      h = scalers[new_scaler].height;
	int bpp = 16; //main_window_tex_format->BitsPerPixel; // TODOSDL2
	 scaler = new_scaler;

	deinit_texture();
	init_texture();

#if defined USE_SDL2
	if (fullscreen_display == -1)
	{
		// Changing scalers, when not in fullscreen mode, forces the window
		// to resize to exactly match the scaler's output dimensions.
		SDL_SetWindowSize(main_window, w, h);
		window_center_in_display(window_get_display_index());
	}
#endif

	fprintf(INFO, "bpp:%d, w:%d x h:%d\n", bpp, vga_width, vga_height);
	fprintf(INFO, "w:%d x h:%d\n", w, h);

	switch (bpp)
	{
	case 32:
		scaler_function = scalers[scaler].scaler32;
		break;
	case 16:
		scaler_function = scalers[scaler].scaler16;
		break;
	default:
		scaler_function = NULL;
		break;
	}

	if (scaler_function == NULL)
	{
		assert(false);
		return false;
	}
        fprintf(INFO, "using scaler:%s\n", scalers[scaler].name);

	return true;
}

bool set_scaling_mode_by_name( const char *name )
{
	for (int i = 0; i < ScalingMode_MAX; ++i)
	{
		 if (strcmp(name, scaling_mode_names[i]) == 0)
		 {
			 scaling_mode = i;
			 return true;
		 }
	}
	return false;
}

void JE_clr256( SDL_Surface *screen )
{
	SDL_FillRect(screen, NULL, 0);
}

void JE_showVGA( void ) 
{ 	
    scale_and_flip(VGAScreen); // draw to main_window_surface!
//  fprintf(stderr, "w:%d x h:%d\n", vga_width, vga_height);
#ifndef USE_SDL2
    // use GLES2 renderer from mySDL.c
    // resample a new texture from rgba_data
    //GLES2_Update_video(main_window_surface);
    GLES2_Update_video(main_window_texture);

    GLES2_Render_video(0);

    #if defined (__ORBIS__)
        orbisGlSwapBuffers();
    #else
        flip_frame();
    #endif

#endif
}

static void calc_dst_render_rect( SDL_Surface *const src_surface, SDL_Rect *const dst_rect )
{
	// Decides how the logical output texture (after software scaling applied) will fit
	// in the window.

	int win_w, win_h;
#if defined USE_SDL2
	SDL_GetWindowSize(main_window, &win_w, &win_h);
#else
        #warning "GL_texture upscaling side-effect here!"
	win_w = 1024, win_h = 768; // XXX GL_texture upscaling side-effect here!
#endif
	int maxh_width, maxw_height;

	switch (scaling_mode)
	{
	case SCALE_CENTER:
//	#if defined USE_SDL2
		SDL_QueryTexture(main_window_texture, NULL, NULL, &dst_rect->w, &dst_rect->h);
//	#endif
		break;
	case SCALE_INTEGER:
		dst_rect->w = src_surface->w;
		dst_rect->h = src_surface->h;
		while (dst_rect->w + src_surface->w <= win_w && dst_rect->h + src_surface->h <= win_h)
		{
			dst_rect->w += src_surface->w;
			dst_rect->h += src_surface->h;
		}
		break;
	case SCALE_ASPECT_8_5:
		maxh_width = win_h * (8.f / 5.f);
		maxw_height = win_w * (5.f / 8.f);

		if (maxh_width > win_w)
		{
			dst_rect->w = win_w;
			dst_rect->h = maxw_height;
		}
		else
		{
			dst_rect->w = maxh_width;
			dst_rect->h = win_h;
		}
		break;
	case SCALE_ASPECT_4_3:
		maxh_width = win_h * (4.f / 3.f);
		maxw_height = win_w * (3.f / 4.f);

		if (maxh_width > win_w)
		{
			dst_rect->w = win_w;
			dst_rect->h = maxw_height;
		}
		else
		{
			dst_rect->w = maxh_width;
			dst_rect->h = win_h;
		}
		break;
	case ScalingMode_MAX:
		assert(false);
		break;
	}

	dst_rect->x = (win_w - dst_rect->w) / 2;
	dst_rect->y = (win_h - dst_rect->h) / 2;
}

static void scale_and_flip( SDL_Surface *src_surface )
{
	assert(src_surface->format->BitsPerPixel == 8);
	// Do software scaling
	assert(scaler_function != NULL);

#if 1 //defined USE_SDL2
	scaler_function(src_surface, main_window_texture);
#else // tested surfaces too
	scaler_function(src_surface, main_window_surface);
#endif

	SDL_Rect dst_rect;
	calc_dst_render_rect(src_surface, &dst_rect);

#if defined USE_SDL2
	// Clear the window and blit the output texture to it
	SDL_SetRenderDrawColor(main_window_renderer, 0, 0, 0, 255);
	SDL_RenderClear(main_window_renderer);
	SDL_RenderCopy(main_window_renderer, main_window_texture, NULL, &dst_rect);
	SDL_RenderPresent(main_window_renderer);
#endif

	// Save output rect to be used by mouse functions
	last_output_rect = dst_rect;
}

/** Converts the given point from the game screen coordinates to the window
 * coordinates, after scaling. */
void map_screen_to_window_pos( int *const inout_x, int *const inout_y )
{
	*inout_x = (*inout_x * last_output_rect.w / VGAScreen->w) + last_output_rect.x;
	*inout_y = (*inout_y * last_output_rect.h / VGAScreen->h) + last_output_rect.y;
}

/** Converts the given point from window coordinates (after scaling) to game
 * screen coordinates. */
void map_window_to_screen_pos( int *const inout_x, int *const inout_y )
{
	*inout_x = (*inout_x - last_output_rect.x) * VGAScreen->w / last_output_rect.w;
	*inout_y = (*inout_y - last_output_rect.y) * VGAScreen->h / last_output_rect.h;
}
