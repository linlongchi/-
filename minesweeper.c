#include "minesweeper.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

static int isInside(const Game *game, int row, int col) {
    return row >= 0 && row < game->rows && col >= 0 && col < game->cols;
}

static void pushHistory(Game *game, int row, int col, char action) {
    MoveNode *node = (MoveNode *)malloc(sizeof(MoveNode));
    if (node == NULL) {
        return;
    }
    node->row = row;
    node->col = col;
    node->action = action;
    node->next = game->history;
    game->history = node;
}

static void placeMines(Game *game) {
    int placed = 0;
    while (placed < game->mines) {
        int row = rand() % game->rows;
        int col = rand() % game->cols;
        if (game->board[row][col].isMine) {
            continue;
        }
        game->board[row][col].isMine = 1;
        placed++;
    }
}

static void calculateNeighborMines(Game *game) {
    int dr[8] = {-1, -1, -1, 0, 0, 1, 1, 1};
    int dc[8] = {-1, 0, 1, -1, 1, -1, 0, 1};

    for (int r = 0; r < game->rows; r++) {
        for (int c = 0; c < game->cols; c++) {
            int count = 0;
            for (int i = 0; i < 8; i++) {
                int nr = r + dr[i];
                int nc = c + dc[i];
                if (isInside(game, nr, nc) && game->board[nr][nc].isMine) {
                    count++;
                }
            }
            game->board[r][c].neighborMines = count;
        }
    }
}

int initGame(Game *game, int rows, int cols, int mines) {
    if (rows <= 0 || cols <= 0 || mines <= 0 || mines >= rows * cols) {
        return 0;
    }

    game->rows = rows;
    game->cols = cols;
    game->mines = mines;
    game->remainingSafe = rows * cols - mines;
    game->gameOver = 0;
    game->history = NULL;

    game->board = (Cell **)malloc(rows * sizeof(Cell *));
    if (game->board == NULL) {
        return 0;
    }

    for (int r = 0; r < rows; r++) {
        game->board[r] = (Cell *)malloc(cols * sizeof(Cell));
        if (game->board[r] == NULL) {
            for (int i = 0; i < r; i++) {
                free(game->board[i]);
            }
            free(game->board);
            game->board = NULL;
            return 0;
        }

        for (int c = 0; c < cols; c++) {
            game->board[r][c].isMine = 0;
            game->board[r][c].isRevealed = 0;
            game->board[r][c].isFlagged = 0;
            game->board[r][c].neighborMines = 0;
        }
    }

    placeMines(game);
    calculateNeighborMines(game);
    return 1;
}

static void floodReveal(Game *game, int startRow, int startCol) {
    int capacity = game->rows * game->cols;
    int *queueRows = (int *)malloc(capacity * sizeof(int));
    int *queueCols = (int *)malloc(capacity * sizeof(int));
    int front = 0;
    int back = 0;
    int dr[8] = {-1, -1, -1, 0, 0, 1, 1, 1};
    int dc[8] = {-1, 0, 1, -1, 1, -1, 0, 1};

    if (queueRows == NULL || queueCols == NULL) {
        free(queueRows);
        free(queueCols);
        return;
    }

    queueRows[back] = startRow;
    queueCols[back] = startCol;
    back++;

    while (front < back) {
        int row = queueRows[front];
        int col = queueCols[front];
        front++;

        if (!isInside(game, row, col)) {
            continue;
        }

        if (game->board[row][col].isRevealed || game->board[row][col].isFlagged) {
            continue;
        }

        game->board[row][col].isRevealed = 1;
        game->remainingSafe--;
        pushHistory(game, row, col, 'R');

        if (game->board[row][col].neighborMines != 0) {
            continue;
        }

        for (int i = 0; i < 8; i++) {
            int nr = row + dr[i];
            int nc = col + dc[i];
            if (isInside(game, nr, nc) && !game->board[nr][nc].isRevealed) {
                queueRows[back] = nr;
                queueCols[back] = nc;
                back++;
            }
        }
    }

    free(queueRows);
    free(queueCols);
}

int revealCell(Game *game, int row, int col) {
    if (!isInside(game, row, col) || game->gameOver) {
        return 0;
    }

    if (game->board[row][col].isRevealed || game->board[row][col].isFlagged) {
        return 0;
    }

    if (game->board[row][col].isMine) {
        game->board[row][col].isRevealed = 1;
        game->gameOver = 1;
        pushHistory(game, row, col, 'X');
        return -1;
    }

    floodReveal(game, row, col);
    return 1;
}

int toggleFlag(Game *game, int row, int col) {
    if (!isInside(game, row, col) || game->gameOver) {
        return 0;
    }

    if (game->board[row][col].isRevealed) {
        return 0;
    }

    game->board[row][col].isFlagged = !game->board[row][col].isFlagged;
    pushHistory(game, row, col, game->board[row][col].isFlagged ? 'F' : 'U');
    return 1;
}

int hasWon(const Game *game) {
    return game->remainingSafe == 0;
}

void printBoard(const Game *game, int revealAll) {
    printf("\n   ");
    for (int c = 0; c < game->cols; c++) {
        printf("%2d ", c);
    }
    printf("\n");

    for (int r = 0; r < game->rows; r++) {
        printf("%2d ", r);
        for (int c = 0; c < game->cols; c++) {
            const Cell *cell = &game->board[r][c];

            if (revealAll && cell->isMine) {
                printf(" * ");
            } else if (cell->isFlagged) {
                printf(" F ");
            } else if (!cell->isRevealed) {
                printf(" . ");
            } else if (cell->neighborMines == 0) {
                printf("   ");
            } else {
                printf(" %d ", cell->neighborMines);
            }
        }
        printf("\n");
    }
}

void printHistory(const Game *game) {
    const MoveNode *current = game->history;
    int count = 0;

    printf("\nRecent moves (latest first):\n");
    while (current != NULL && count < 10) {
        printf("- (%d, %d) action=%c\n", current->row, current->col, current->action);
        current = current->next;
        count++;
    }
    if (count == 0) {
        printf("- no moves yet\n");
    }
}

void freeGame(Game *game) {
    if (game->board != NULL) {
        for (int r = 0; r < game->rows; r++) {
            free(game->board[r]);
        }
        free(game->board);
        game->board = NULL;
    }

    while (game->history != NULL) {
        MoveNode *next = game->history->next;
        free(game->history);
        game->history = next;
    }
}
