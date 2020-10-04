#include <sys/types.h> // u_char and friends
//#include <SDL.h>
//#include <SDL2_gfxPrimitives.h>

// Clang Extended Vectors
typedef float vec2 __attribute__((ext_vector_type(2)));
typedef float vec4 __attribute__((ext_vector_type(4)));

int initGraph(char *title);

void freeResources();

void clearScreen();

void updateScreen();

void rect(int x1, int y1, int x2, int y2, u_char r, u_char g, u_char b);

void filledRect(int x1, int y1, int x2, int y2, u_char r, u_char g, u_char b);

void textOut(int x, int y, char *s, u_char r, u_char g, u_char b);

int screenWidth();

int screenHeight();

int getKey();

//vec2 px_pos_to_normalized(vec2 *pos, vec2 *size);
vec4 px_pos_to_normalized2(vec2 *pos, vec2 *size);
void ORBIS_RenderDrawLines(const vec2 *points, int count);
void ORBIS_RenderFillRects(const vec4 *rects,  int count);
