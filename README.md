# Checkers

An interactive single-file C++ implementation of American checkers. The game runs in the terminal, prints the board after each turn, and enforces the core rules of checkers including captures, king promotion, and multi-jump turns.

## Features

- Standard 8x8 board with coordinate labels.
- Red and White pieces with legal forward movement.
- Mandatory captures when a capture is available.
- Multi-jump capture chains with the same piece.
- King promotion on the back rank.
- Win detection when a player has no legal moves.
- Draw detection after 40 moves per side without a capture or promotion.

## How To Run

This project is a single source file: [checkers.cpp](checkers.cpp).

### Build

On Windows with MinGW g++:

```bash
g++ -std=c++17 -O2 -Wall -Wextra checkers.cpp -o checkers.exe
```

### Run

```bash
checkers.exe
```

## How To Play

1. The game starts with Red to move.
2. Enter the row and column of the piece you want to move.
3. Enter the destination row and column.
4. If a capture is available, you must take it.
5. If another capture is available with the same piece, you must continue jumping.

## Input Format

All coordinates are zero-based:

- Rows: `0` to `7`
- Columns: `0` to `7`

Example move input:

```text
5 0
4 1
```

## Rules Implemented

- Red pieces move upward toward row `0`.
- White pieces move downward toward row `7`.
- Kings can move and capture in both diagonal directions.
- Captures jump over one opposing piece into an empty square.
- Promotion happens automatically when a piece reaches the far edge.

## Notes

- The game is console-based and does not use a graphical interface.
- If input ends unexpectedly, the program exits gracefully.