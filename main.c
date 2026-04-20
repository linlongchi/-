#include "minesweeper.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

static void printMenu(void) {
    printf("\nCommands:\n");
    printf("- r <row> <col> : reveal cell\n");
    printf("- f <row> <col> : toggle flag\n");
    printf("- h             : show move history\n");
    printf("- q             : quit game\n");
}

int main(void) {
    Game game;
    int rows = 9;
    int cols = 9;
    int mines = 10;
    char command;
    int row;
    int col;

    srand((unsigned int)time(NULL));

    if (!initGame(&game, rows, cols, mines)) {
        printf("Failed to initialize game.\n");
        return 1;
    }

    printf("=== Minesweeper (C Project) ===\n");
    printf("Board: %dx%d, Mines: %d\n", rows, cols, mines);
    printMenu();

    while (!game.gameOver && !hasWon(&game)) {
        printBoard(&game, 0);
        printf("\nEnter command: ");

        if (scanf(" %c", &command) != 1) {
            printf("Input error.\n");
            break;
        }

        if (command == 'q') {
            break;
        } else if (command == 'h') {
            printHistory(&game);
            continue;
        } else if (command == 'r' || command == 'f') {
            if (scanf("%d %d", &row, &col) != 2) {
                printf("Invalid format. Example: r 3 4\n");
                continue;
            }

            if (command == 'r') {
                int result = revealCell(&game, row, col);
                if (result == -1) {
                    printf("Boom! You hit a mine.\n");
                } else if (result == 0) {
                    printf("Cannot reveal that cell.\n");
                }
            } else {
                if (!toggleFlag(&game, row, col)) {
                    printf("Cannot toggle flag there.\n");
                }
            }
        } else {
            printf("Unknown command.\n");
            printMenu();
        }
    }

    if (hasWon(&game)) {
        printf("\nCongratulations! You cleared all safe cells.\n");
    } else if (game.gameOver) {
        printf("\nGame over.\n");
    } else {
        printf("\nGame exited.\n");
    }

    printBoard(&game, 1);
    freeGame(&game);
    return 0;
}
