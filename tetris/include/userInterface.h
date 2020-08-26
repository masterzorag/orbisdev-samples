#ifndef TETRIS__USER_INTERFACE_H
#define TETRIS__USER_INTERFACE_H

#include <sys/types.h> // u_char and friends

#include "game.h"

#if defined (__ORBIS__)
#define SQUARE_WIDTH 30
#else
#define SQUARE_WIDTH 20
#endif

void drawField(int field[F_COLS][F_ROWS]);
void drawControls();
void drawNextPiece(int nextPiece);
void drawEndGame(int score);
void drawScore(int score);

#endif // TETRIS__USER_INTERFACE_H
