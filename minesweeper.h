#ifndef MINESWEEPER_H
#define MINESWEEPER_H

typedef struct Cell {
    int isMine;
    int isRevealed;
    int isFlagged;
    int neighborMines;
} Cell;

typedef struct MoveNode {
    int row;
    int col;
    char action;
    struct MoveNode *next;
} MoveNode;

typedef struct Game {
    int rows;
    int cols;
    int mines;
    int remainingSafe;
    int gameOver;
    Cell **board;
    MoveNode *history;
} Game;

int initGame(Game *game, int rows, int cols, int mines);
void freeGame(Game *game);
void printBoard(const Game *game, int revealAll);
int revealCell(Game *game, int row, int col);
int toggleFlag(Game *game, int row, int col);
int hasWon(const Game *game);
void printHistory(const Game *game);

#endif
