#ifndef TETRIS__GAME_H
#define TETRIS__GAME_H

#if defined (__ORBIS__)

#include <ps4sdk.h>
#include <debugnet.h>
#define  fprintf  debugNetPrintf
#define  ERROR    DEBUGNET_ERROR
#define  DEBUG    DEBUGNET_DEBUG
#define  INFO     DEBUGNET_INFO
#include <orbisGl.h>

#else

#include <stdio.h>
#define  debugNetPrintf  fprintf
#define  ERROR           stderr
#define  DEBUG           stdout
#define  INFO            stdout

#endif // defined (__ORBIS__)

#define DELAY 16
#define F_COLS 10
#define F_ROWS 20

#define EMPTY 0
#define PLAYER 1
#define FILLED 2

#define SPEED_SLOW 1
#define SPEED_MEDIUM 2
#define SPEED_FAST 4

int play();
void initialize();
int whatIsInside();
void initField();
void movePiece(int);
void moveLeft();
void moveRight();
void rotatePiece();
void checkIfLine();
void endGame();
int setNewPieceInField(int);
int moveDown();

#endif // TETRIS__GAME_H
