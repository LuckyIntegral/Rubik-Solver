# Rubik Solver

Rubik Solver is a C++ command-line solver with a Python desktop visualizer built with pygame.

The repository includes:
- A C++ binary named `rubik` that parses a scramble and prints a solution sequence.
- A Python visualizer (`visualizer/`) that renders the cube in 3D, runs the solver, and animates moves.

## Project Overview

Main parts:
- `srcs/`, `incs/`: C++ solver source and headers.
- `visualizer/`: Python frontend (pygame UI and 3D scene rendering).
- `requirements.txt`: Python dependencies.
- `Makefile`: Build and clean targets for the C++ binary.

## Prerequisites

- C++ compiler with Make support (`g++`, `make`)
- Python 3.9+

## Build The C++ Solver

From project root:

```bash
make
```

This creates the executable:

```bash
./rubik
```

Useful targets:

```bash
make clean
make fclean
make re
```

## C++ Solver Usage

Moves-only mode:

```bash
./rubik "R U R' U'"
```

Algorithm-select mode:

```bash
./rubik -a algoA "R U R' U'"
./rubik -a algoB "R U R' U'"
```

Notes:
- The executable name is not counted as an argument.
- Supported algorithm names are currently `algoA` and `algoB`.
- Moves must be space-separated and use standard notation (`R`, `U'`, `F2`, ...).

## Python Environment (venv)

Create a virtual environment in the project root:

```bash
python3 -m venv venv
```

Activate it:

```bash
source venv/bin/activate
```

Deactivate when done:

```bash
deactivate
```

## Install Python Packages

After activating `venv`, install dependencies:

```bash
pip install -r requirements.txt
```

If dependencies change later, run the same command again to sync your environment.

## Run The Visualizer

From project root, with `venv` activated:

```bash
python3 -m visualizer
```

Alternative:

```bash
python3 visualizer/main.py
```

## Visualizer Usage

Main workflow:
1. Enter scramble moves in the scramble input.
2. Optionally set scramble length and click `Generate scramble`.
3. Choose algorithm in the algorithm dropdown (`algoA` or `algoB`).
4. Click `Run ./rubik` to call the C++ solver.
5. Use `Load sequence`, `Play`, `Step`, `Back`, `Scramble only`, and `Solve only` for playback.

Controls:
- Mouse drag on canvas: rotate camera
- Mouse wheel: zoom
- `SPACE`: play/pause
- `LEFT` / `RIGHT`: step back/forward
- `R`: reset cube
- `V`: reset camera
- `ESC`: quit

## Troubleshooting

`./rubik: No such file or directory`
- Build first with `make`.

`Solver is not executable`
- Run `chmod +x ./rubik`.

`No module named pygame` (or similar)
- Activate `venv` and run `pip install -r requirements.txt`.

`Invalid move: ...`
- Check notation and spacing in the move sequence.
