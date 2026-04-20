# C Language Final Project - Minesweeper (Flask + C)

This project follows the "AI x system integration" requirement:
- Browser frontend (Flask rendered web page)
- Python Flask routing layer
- C executable as backend core logic (called via `subprocess`)

It also demonstrates:
- pointer operations
- dynamic memory management (`malloc` / `free`)
- `struct`-based modular design
- function decomposition with clear responsibilities
- data structure usage (dynamic 2D array + linked list history)

## Features

- Difficulty levels (easy / medium / hard)
- Reveal cell and toggle flag from web UI
- Win/loss detection
- Auto-expand area when revealing cells with 0 neighboring mines
- C backend engine invoked per action by Flask subprocess

## Project Structure

- `app.py`: Flask routes and web integration
- `templates/index.html`: browser UI
- `c_backend_api.c`: C command API for Flask subprocess calls
- `minesweeper.h`: data structures and API declarations
- `minesweeper.c`: game logic, memory management, and board operations
- `main.c`: optional terminal-only version

## Core C Concepts Demonstrated

### 1. Pointer
- Access dynamic 2D board through `Cell **board`
- Pointer traversal for board operations and linked list history

### 2. Dynamic Memory (`malloc` / `free`)
- Board allocation:
  - `board = malloc(rows * sizeof(Cell *))`
  - `board[i] = malloc(cols * sizeof(Cell))`
- Queue allocation in flood reveal
- Linked-list node allocation for history
- Full deallocation in reverse lifecycle (`freeGame`)

### 3. Struct Design
- `Cell`: mine/reveal/flag/neighbor state
- `Game`: global game context and resources
- `MoveNode`: linked-list history records

### 4. Function Decomposition
- `initGame`, `freeGame`
- `revealCell`, `toggleFlag`
- `printBoard`, `printHistory`, `hasWon`

## Architecture

Browser -> Flask (`app.py`) -> subprocess -> `mines_backend.exe` (C core) -> Flask -> Browser

## How to Run

### 1) Compile C backend (GCC / MinGW)
```bash
gcc -Wall -Wextra -std=c11 c_backend_api.c minesweeper.c -o mines_backend.exe
```

### 2) Install Python package
```bash
pip install flask
```

### 3) Start Flask app
```powershell
python app.py
```

### 4) Open browser

- [http://127.0.0.1:5000](http://127.0.0.1:5000)

## Web Controls

- New Easy / Medium / Hard
- Tool selector: Reveal / Flag
- Click cell to perform selected action

## Cursor Prompt Usage Record (Sample)

1. "Design Minesweeper architecture with struct and function split, do not write full code in one block."
2. "Implement dynamic 2D array allocation and deallocation safely."
3. "Check for memory leak / pointer out-of-bounds risks in reveal logic."
4. "Explain why free order must mirror malloc order in this project."

## Debug and Safety Notes

- Avoid out-of-bounds by validating row/col with `isInside`.
- Reject reveal on flagged/revealed cells.
- Ensure every `malloc` has a matching `free`.
- Free all linked-list history nodes before exit.

## Rubric Alignment (PDF)

- Functionality: playable web minesweeper
- C Core: pointer + malloc/free used throughout backend
- Data Structure: dynamic 2D array and linked list
- Architecture: Browser + Flask + C subprocess layered design
- Documentation: this README includes design and run instructions
