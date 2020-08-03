#include <stdio.h>

//#include <SDL.h>
//#include <SDL2_gfxPrimitives.h>

//#include "audio/audio.h"
#include "primlib.h"
/*
static SDL_Window *sdlWindow;
static SDL_Renderer *sdlRenderer;
*/

#if defined (__PS4__)
#include <debugnet.h>
#endif


typedef struct
{
    float x;
    float y;
} SDL_FPoint;

typedef struct
{
    float h;
    float w;
    float x;
    float y;
} SDL_FRect;

// Clang Extended Vectors
typedef float vec2 __attribute__((ext_vector_type(2)));
typedef float vec4 __attribute__((ext_vector_type(4)));
vec4 px_pos_to_normalized(vec2 *pos, vec2 *size);
//int ORBIS_RenderFillRects(onst SDL_FRect *rects, int count);

extern vec4 color; // from rect.c

void rect(int x1, int y1, int x2, int y2, u_char r, u_char g, u_char b) {
  //rectangleRGBA(sdlRenderer, x1, y1, x2 + 1, y2 + 1, r, g, b, SDL_ALPHA_OPAQUE);
    color.r = r, color.g = g, color.b = b, color.a = 255;
    /* normalize 0-1 */
    color /= 255.f;
    /* grab position and *size* in px! */
    vec2 p = (vec2) {      x1,      y1 },
         s = (vec2) { x2 - x1, y2 - y1 };
    /* convert to normalized coordinates! */
    vec4 n = px_pos_to_normalized( &p, &s );

    n.zw += n.xy; // turn size into p2

    // test lines
    SDL_FPoint v[4];
    v[0].x = n.x, v[0].y = n.y;  v[1].x = n.z, v[1].y = n.y;
    v[2].x = n.x, v[2].y = n.w;  v[3].x = n.z, v[3].y = n.w;
    ORBIS_RenderDrawLines(&v[0], 6); // horiz

    v[0].x = n.x, v[0].y = n.y;  v[1].x = n.x, v[1].y = n.w;
    v[2].x = n.z, v[2].y = n.y;  v[3].x = n.z, v[3].y = n.w;
    ORBIS_RenderDrawLines(&v[0], 6); // vert
}

void filledRect(int x1, int y1, int x2, int y2, u_char r, u_char g, u_char b) {
  //boxRGBA(sdlRenderer, x1, y1, x2, y2, r, g, b, SDL_ALPHA_OPAQUE);
//debugNetPrintf(3, "%s: %d %d %d %d %u %u %u\n", __FUNCTION__, x1, y1, x2, y2, r, g, b);
  color.r = r, color.g = g, color.b = b, color.a = 255;
  /* normalize 0-1 */
  color /= 255.f;
  /* grab position and *size* in px! */
  vec2 p = (vec2) {      x1,      y1 },
       s = (vec2) { x2 - x1, y2 - y1 };
  /* convert to normalized coordinates! */
  vec4 n = px_pos_to_normalized( &p, &s );
  /* fill the SDL_FRect data */
  SDL_FRect rect;
  rect.x = n.x, rect.y = n.y;
  rect.w = n.z, rect.h = n.w;
  /* GLES2 draw */
  ORBIS_RenderFillRects(&rect, 1);
/*
  debugNetPrintf(3,"%s: pos:  %f,%f size:%f,%f\n", __FUNCTION__, n.x, n.y, n.w, n.w);
  debugNetPrintf(3,"color:%f,%f,%f,%f\n", color.r, color.g, color.b, color.a);
*/
}

int screenWidth()  { return 1920; }
int screenHeight() { return 1080; }

#if 0
void clearScreen() {
  SDL_SetRenderDrawColor(sdlRenderer, 10, 10, 10, SDL_ALPHA_OPAQUE);
  SDL_RenderClear(sdlRenderer);
}

void updateScreen() { SDL_RenderPresent(sdlRenderer); }

int screenWidth() {
  int w;
  int h;
  SDL_GetWindowSize(sdlWindow, &w, &h);
  return w;
}

int screenHeight() {
  int w;
  int h;
  SDL_GetWindowSize(sdlWindow, &w, &h);
  return h;
}

void textOut(int x, int y, char *s, u_char r, u_char g, u_char b) {
  stringRGBA(sdlRenderer, x, y, s, r, g, b, 255);
}

int getKey() {
  SDL_Event e;
  while (SDL_PollEvent(&e)) {
    if (e.type == SDL_QUIT) {
      return -1;
    }
    if (e.type == SDL_KEYDOWN) {
      return e.key.keysym.sym;
    }
  }

  return 0;
}

int initGraph(char *title) {
  if (sdlWindow) {
    fprintf(stderr, "initGraph called twice\n");
    return 1;
  }

  // Initialize SDL
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
    printf("SDL could not be initialized! SDL_Error: %s\n", SDL_GetError());
    return 1;
  }

  // Create window
  sdlWindow = SDL_CreateWindow(title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 800, 600,
                               SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);

  if (sdlWindow == NULL) {
    fprintf(stderr, "Window could not be created! SDL_Error: %s\n", SDL_GetError());
    return 1;
  }

  /// list sdl video drivers
    int i =0, rd=0;
    SDL_RendererInfo info;
    rd = SDL_GetNumRenderDrivers();
    while ( i < rd )
    {
        if ( !SDL_GetRenderDriverInfo(i, &info) )
        {   //SDL_Log("%d: %s\n", i, info.name);
            SDL_Log("%d: %s\n", i, info.name);
        }
        i++;
    }

  // select opengles2
  sdlRenderer = SDL_CreateRenderer(sdlWindow, 1, 0);
  if (sdlRenderer == NULL) {
    fprintf(stderr, "Error when obtaining the renderer, SDL_Error: %s\n", SDL_GetError());
    return 1;
  }

  if(sdlRenderer) {
        SDL_RendererInfo renderer_info = {0} ;
        if (!SDL_GetRendererInfo(sdlRenderer, &renderer_info))
            SDL_Log("Initialized %s renderer at %p\n", renderer_info.name, sdlRenderer);
  }

  initAudio();

  return 0;
}

void freeResources() {
  SDL_DestroyWindow(sdlWindow);
  SDL_Quit();
}
#endif
