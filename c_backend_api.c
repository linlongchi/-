#include "minesweeper.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static int allocateBoard(Game *game, int rows, int cols) {
    game->rows = rows;
    game->cols = cols;
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
    }
    return 1;
}

static int readStateFromStdin(Game *game) {
    int rows;
    int cols;
    int mines;
    int gameOver;
    int remainingSafe;

    if (scanf("%d %d %d %d %d", &rows, &cols, &mines, &gameOver, &remainingSafe) != 5) {
        return 0;
    }

    memset(game, 0, sizeof(Game));
    game->mines = mines;
    game->gameOver = gameOver;
    game->remainingSafe = remainingSafe;
    game->history = NULL;

    if (!allocateBoard(game, rows, cols)) {
        return 0;
    }

    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
            Cell *cell = &game->board[r][c];
            if (scanf("%d %d %d %d",
                      &cell->isMine,
                      &cell->isRevealed,
                      &cell->isFlagged,
                      &cell->neighborMines) != 4) {
                freeGame(game);
                return 0;
            }
        }
    }
    return 1;
}

static void printState(const Game *game, int resultCode) {
    int won = hasWon(game);
    int exploded = resultCode == -1;

    printf("STATUS %d %d %d %d %d %d %d %d\n",
           game->rows,
           game->cols,
           game->mines,
           game->remainingSafe,
           game->gameOver,
           won,
           exploded,
           resultCode);
    printf("BOARD\n");
    for (int r = 0; r < game->rows; r++) {
        for (int c = 0; c < game->cols; c++) {
            const Cell *cell = &game->board[r][c];
            printf("%d %d %d %d\n",
                   cell->isMine,
                   cell->isRevealed,
                   cell->isFlagged,
                   cell->neighborMines);
        }
    }
    printf("END\n");
}

int main(int argc, char *argv[]) {
    Game game;
    memset(&game, 0, sizeof(Game));

    if (argc < 2) {
        fprintf(stderr, "Usage: mines_backend <new|act> ...\n");
        return 1;
    }

    if (strcmp(argv[1], "new") == 0) {
        int rows;
        int cols;
        int mines;
        unsigned int seed;

        if (argc < 5) {
            fprintf(stderr, "Usage: mines_backend new <rows> <cols> <mines> [seed]\n");
            return 1;
        }

        rows = atoi(argv[2]);
        cols = atoi(argv[3]);
        mines = atoi(argv[4]);
        seed = (argc >= 6) ? (unsigned int)strtoul(argv[5], NULL, 10) : (unsigned int)time(NULL);
        srand(seed);

        if (!initGame(&game, rows, cols, mines)) {
            fprintf(stderr, "initGame failed\n");
            return 1;
        }
        printState(&game, 1);
        freeGame(&game);
        return 0;
    }

    if (strcmp(argv[1], "act") == 0) {
        char action;
        int row;
        int col;
        int result = 0;

        if (argc < 5) {
            fprintf(stderr, "Usage: mines_backend act <r|f> <row> <col>\n");
            return 1;
        }

        action = argv[2][0];
        row = atoi(argv[3]);
        col = atoi(argv[4]);

        if (!readStateFromStdin(&game)) {
            fprintf(stderr, "Failed to read game state from stdin\n");
            return 1;
        }

        if (action == 'r') {
            result = revealCell(&game, row, col);
        } else if (action == 'f') {
            result = toggleFlag(&game, row, col);
        } else {
            freeGame(&game);
            fprintf(stderr, "Unknown action: %c\n", action);
            return 1;
        }

        if (hasWon(&game)) {
            game.gameOver = 1;
        }

        printState(&game, result);
        freeGame(&game);
        return 0;
    }

    fprintf(stderr, "Unknown mode: %s\n", argv[1]);
    return 1;
}
