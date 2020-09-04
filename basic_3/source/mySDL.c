/*
    minimal SDL part to blit on RGB565 bitmap
    surface/texture using a software renderer

    this code is part of SDL2 ripped for the job;
    some edits to run on orbis, timers and raster
    surface/texture: we will use a GLES2 renderer

    2020, masterzorag
*/


#include <stdlib.h> // calloc
#include <string.h> // memcpy, memset, strlen
#include <sys/time.h>
#include <sys/select.h>
#include <errno.h>
#include <stdint.h>
#include <stdbool.h>

#include "mySDL.h" // custom: minimal for the raster/timing job


#if defined (__ORBIS__)

#include <ps4sdk.h>
#include <debugnet.h>
#define  fprintf  debugNetPrintf
#define  ERROR    DEBUGNET_ERROR
#define  DEBUG    DEBUGNET_DEBUG
#define  INFO     DEBUGNET_INFO

#else // on linux

#include <stdio.h>
#include <string.h>
#define  debugNetPrintf  fprintf
#define  ERROR           stderr
#define  DEBUG           stdout
#define  INFO            stdout

#endif // (__ORBIS__)


#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))


/* unix timers using nanosleep */
static struct timeval start_tv;
static bool   ticks_started = false;

void
SDL_TicksInit(void) // once
{
    if(ticks_started) return;

    gettimeofday(&start_tv, NULL);
    ticks_started = true;
}

uint32_t
SDL_GetTicks(void) // get timing
{
    if(!ticks_started) SDL_TicksInit();

    struct timeval now;
    gettimeofday(&now, NULL);
    uint32_t ticks = (uint32_t)((now.tv_sec  - start_tv.tv_sec ) * 1000
                              + (now.tv_usec - start_tv.tv_usec) / 1000);
    return ticks;
}

#if 1 // yes we HAVE_NANOSLEEP
void
SDL_Delay(uint32_t ms)
{
    int    was_error;
    struct timespec elapsed, tv;
    /* Set the timeout interval */
    elapsed.tv_sec  = ms / 1000;
    elapsed.tv_nsec = (ms % 1000) * 1000000;
    do {
        errno      = 0;
        tv.tv_sec  = elapsed.tv_sec;
        tv.tv_nsec = elapsed.tv_nsec;
        was_error  = nanosleep(&tv, &elapsed);
    } while (was_error && (errno == EINTR));
//  fprintf(INFO, "elapsed %u ms\n", ms);
}
#else // for reference
void
SDL_Delay(uint32_t ms)
{
    int      was_error;
    struct   timeval tv;
    uint32_t then, now, elapsed;
    /* Set the timeout interval */
    then = SDL_GetTicks();//SDL_GetTicks();
    do {
        errno   = 0;
        /* Calculate the time interval left (in case of interrupt) */
        now     = SDL_GetTicks();
        elapsed = (now - then);
        then    = now;
        if(elapsed >= ms) break;
        ms        -= elapsed;
        tv.tv_sec  = ms / 1000;
        tv.tv_usec = (ms % 1000) * 1000;
        was_error  = select(0, NULL, NULL, NULL, &tv);
    } while (was_error && (errno == EINTR));
    fprintf(INFO, "elapsed:%u > %u\n", elapsed, ms);
}
#endif

size_t
SDL_strlcpy(char *dst, const char *src, size_t maxlen)
{
    size_t srclen = strlen(src);
    if (maxlen > 0) {
        size_t len = MIN(srclen, maxlen - 1);
        memcpy(dst, src, len);
        dst[len] = '\0';
    }
    return srclen;
}

/* raster impementation! */
static void
SDL_FillRect1(uint8_t *pixels, int pitch, uint32_t color, int w, int h)
{
    while (h--) {
        memset(pixels, color, w);
        pixels += pitch;
    }
}

// seems not used, but for reference
static void
SDL_FillRect2(uint8_t * pixels, int pitch, uint32_t color, int w, int h)
{
    int n;
    uint16_t *p = NULL;
    
    while (h--) {
        n = w;
        p = (uint16_t *) pixels;
        if (n > 1) {
            if ((uintptr_t) p & 2) {
                *p++ = (uint16_t) color;
                --n;
            }
            memset(p, color, (n >> 1));
        }
        if (n & 1) { p[n - 1] = (uint16_t) color; }
        pixels += pitch;
    }
}

/* raster impementation! */
int
SDL_FillRects(SDL_Surface *dst, const SDL_Rect *rects, int count, uint32_t color)
{
    uint8_t *pixels;
    const SDL_Rect* rect;
    void (*fill_function)(uint8_t * pixels, int pitch, uint32_t color, int w, int h) = NULL;
    int i;
    switch (dst->format->BytesPerPixel)
    {
        case 1: { color |= (color << 8);
                  color |= (color << 16); fill_function = SDL_FillRect1; break; }
        case 2: { color |= (color << 16); fill_function = SDL_FillRect2; break; }
    }
    for (i = 0; i < count; ++i)
    {
        rect   = &rects[i];
        pixels = (uint8_t *)dst->pixels + rect->y * dst->pitch +
                                          rect->x * dst->format->BytesPerPixel;
        fill_function(pixels, dst->pitch, color, 
                              rect->w, rect->h);
    }
    return 0;
}

/* raster impementation! */
int
SDL_FillRect(SDL_Surface *dst, const SDL_Rect *rect, uint32_t color)
{
    /* If 'rect' == NULL, then fill the whole surface */
    if (!rect) {
        rect = &dst->clip_rect;
        // but skip if clip rect is empty!
        if(rect->x == rect->y == 0
        && rect->w == rect->h == 0) return 0;
    }
//  printf("%d,%d - %d,%d: %8x %d\n", rect->x, rect->y, rect->w, rect->h, color, dst->pitch);
    return SDL_FillRects(dst, rect, 1, color);
}

// setup an SDL_PIXELFORMAT_RGB565 SDL_PixelFormat
SDL_PixelFormat *
SDL_AllocFormat(uint32_t pixel_format)
{
    /* Allocate a pixel format structure, and initialize it */
    SDL_PixelFormat *p = calloc(1, sizeof(*p));
    if (!p) return NULL;

    switch(pixel_format)
    {
      case 318769153: // Bpp: 8bits per pixel -> indexed palette colors!
        p->format       = 318769153, p->palette       = NULL, 
    //  SDL_Palette *palette is NULL by calloc
        p->BitsPerPixel = 8,         p->BytesPerPixel = 1,
    //  p->padding = 0, 
    //  p->Rmask  = 0,   p->Gmask  = 0, p->Bmask  = 0, p->Amask = 0;
        p->Rloss  = 8,   p->Gloss  = 8, p->Bloss  = 8, p->Aloss = 8; 
    //  p->Rshift = 0,   p->Gshift = 0, p->Bshift = 0, p->Ashift = 0;
        p->refcount = 1, p->next = NULL;
        break;

      case 353701890: // Bpp: SDL_PIXELFORMAT_RGB565 !!! (RGB565 from 16bits, 2bytes)
        p->format       = 353701890, p->palette       = NULL,
        p->BitsPerPixel = 16,        p->BytesPerPixel = 2,
    //  p->padding[2];
        p->Rmask  = 63488, p->Gmask  = 2016, p->Bmask  = 31, p->Amask  = 0;
        p->Rloss  =     3, p->Gloss  =    2, p->Bloss  =  3, p->Aloss  = 8;
        p->Rshift =    11, p->Gshift =    5, p->Bshift =  0, p->Ashift = 0;
        p->refcount =   1, p->next   = NULL;
        break;
    }
    return p;
}

void
SDL_FreeFormat(SDL_PixelFormat *format)
{
    if (!format) return;

    if (format->palette) free(format->palette), format->palette = NULL;

    free(format), format = NULL;
}

/*
 * Create an empty RGB surface of the appropriate depth
 */
SDL_Surface *
SDL_CreateRGBSurface(uint32_t flags,
                     int width, int height, int depth,
                     uint32_t Rmask, uint32_t Gmask, uint32_t Bmask, uint32_t Amask)
{
    fprintf(INFO, "%s, depth:%u\n", __FUNCTION__, depth); // depth color of 1 pixel! (8, 16, 32)
	/* Allocate the surface */
    SDL_Surface *surface = calloc(1, sizeof(*surface));
    if (!surface) return NULL;

    switch(depth)
    {
      case  8: surface->format = SDL_AllocFormat(318769153); break;
      case 16: surface->format = SDL_AllocFormat(353701890); break;
      default: fprintf(ERROR, "unimplemented bit depth!\n"); break;
    }
    surface->pixels      = calloc(width * height, surface->format->BytesPerPixel);
    surface->w           = width, surface->h = height,
    surface->pitch       = (int)width;
    surface->flags       = 8,     surface->userdata    = NULL,
    surface->locked      = 0,     surface->lock_data   = NULL,  
    surface->clip_rect.x = 0,     surface->clip_rect.y = 0,
    surface->clip_rect.w = width, surface->clip_rect.h = height,
    surface->map         = NULL,  surface->refcount    = 1;

    return surface;
}

SDL_Texture *
SDL_CreateTexture(SDL_Renderer *renderer, uint32_t format, int access, int w, int h)
{
    if (w <= 0 || h <= 0) { fprintf(ERROR, "Texture dimensions can't be 0"); return NULL; }

    SDL_Texture *texture = calloc(1, sizeof(*texture));
    if (!texture) return NULL;

    texture->format = format;
    texture->w = w;
    texture->h = h;

    int BytePerPixel = 0;
    switch(format)
    {
      case 318769153: BytePerPixel = 1; break;
      case 353701890: BytePerPixel = 2; break;
      default: fprintf(ERROR, "unimplemented format!\n");  break;
    }
    /* The pitch is 4 byte aligned */
    texture->pitch  = (((w * BytePerPixel) + 3) & ~3);
    texture->pixels = calloc(1, texture->pitch * h);

    fprintf(INFO, "%s, w:%d, h:%d, pitch:%u\n", __FUNCTION__, texture->w, texture->h, texture->pitch); // depth color of 1 pixel! (8, 16, 32)

    return texture;
}

uint32_t SDL_MapRGB(SDL_PixelFormat *format, uint8_t r, uint8_t g, uint8_t b)
{
    if (format->palette == NULL) {
        return (r >> format->Rloss) << format->Rshift
             | (g >> format->Gloss) << format->Gshift
             | (b >> format->Bloss) << format->Bshift
             | format->Amask;
    } else {
        // SDL_FindColor(format->palette, r, g, b, SDL_ALPHA_OPAQUE);
        return 0xFFFFFFFF;
    }
}

void SDL_Quit(void) { }

void SDL_GetError(void)
{
    fprintf(ERROR, "%s: %s\n", __FUNCTION__, strerror(errno));
}

void SDL_LockTexture(SDL_Texture *texture, const SDL_Rect *rect, void **pixels, int *pitch)
{
    /* mutex :wen:? */
    *pixels = texture->pixels;
    *pitch  = texture->pitch;
}

void SDL_UnlockTexture(SDL_Texture *texture) {/* mutex :wen:? */}


void SDL_QueryTexture(SDL_Texture *texture, void *u1, void *u2, int *width, int *height)
{
    *width  = texture->w;
    *height = texture->h;
}

/* stubbed out follows: */

void SDL_InitSubSystem(void) { }
void SDL_QuitSubSystem(void) { }
void SDL_WasInit(void) { }
void SDL_GetScancodeName(void) { }

void SDL_JoystickName(void) { }
void SDL_JoystickGetButton(void) { }
void SDL_JoystickGetAxis(void) { }
void SDL_JoystickGetHat(void) { }
void SDL_JoystickNumButtons(void) { }
void SDL_JoystickUpdate(void) { }

void SDL_PushEvent(void) { }
void SDL_PollEvent(void) { }

void SDL_NumJoysticks(void) { }
void SDL_JoystickOpen(void) { }
void SDL_JoystickNumHats(void) { }
void SDL_JoystickNumAxes(void) { }
void SDL_JoystickEventState(void) { }
void SDL_GetScancodeFromName(void) { }
void SDL_GetModState(void) { }
void SDL_ShowCursor(void) { }
void SDL_GetKeyName(void) { }
void SDL_JoystickClose(void) { }

// custom breakpoint
void my_break(void)
{
    //
}
