#include <sys/types.h> // u_char and friends
//#include <SDL.h>
//#include <SDL2_gfxPrimitives.h>

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
