# Rubik Solver

Rubik Solver is a C++ command-line solver with a Python desktop visualizer built with pygame.

The repository includes:

- A C++ binary named `rubik` that parses a scramble and prints a solution sequence.
- A Python visualizer (`visualizer/`) that renders the cube in 3D, runs the solver, and animates moves.

---

## Warning for 42 Students

This repository is intended as a reference and educational tool. **42 students are strongly advised not to copy this code without fully understanding its functionality.** Plagiarism in any form is against 42's principles and could lead to serious academic consequences. Use this repository responsibly to learn and better understand how to implement similar functionalities on your own.

## Project Overview

Main parts:

- `srcs/`, `incs/`: C++ solver source and headers.
- `visualizer/`: Python frontend (pygame UI and 3D scene rendering).
- `requirements.txt`: Python dependencies.
- `Makefile`: Build targets for the solver, tests, and the visualizer environment.

## Prerequisites

- C++ compiler with Make support (`g++`, `make`)
- Python 3.9+ (for the visualizer)

## Build The C++ Solver

From project root:

```bash
make
```

This creates the executable:

```bash
./rubik
```

### Makefile targets

| Target | Description |
|--------|-------------|
| `make` / `make all` | Build `./rubik`. |
| `make clean` | Remove object files (`objs/`). |
| `make fclean` | Remove `objs/`, `rubik`, and test binaries (`test`, `test_performance`). Does **not** remove the Python `venv/`. |
| `make re` | `fclean` the main binary and objects, then rebuild `rubik`. |
| `make run` | Rebuild `rubik` and run `./rubik` (no scramble; use `./rubik "moves"` for a real run). |
| `make test` | Build and run the **phase stress test** (see below). |
| `make test_performance` | Build and run the **performance demo** (see below). |
| `make v` | Ensure `venv/` exists, install Python deps if needed, run `visualizer/main.py`. |
| `make clean-venv` | Delete the `venv/` directory (next `make v` recreates it). |

## C++ Solver Usage

Default: print the solution as one move per line:

```bash
./rubik "R U R' U'"
```

Human-readable report (phases, timings, colors):

```bash
./rubik --human "R U R' U'"
./rubik -h "R U R' U'"
```

Performance-oriented report (`performance_solution()`):

```bash
./rubik --performance "R U R' U'"
./rubik -p "R U R' U'"
```

Notes:

- With flags, the scramble string is the **last** argument.
- Moves must be space-separated and use standard notation (`R`, `U'`, `F2`, …).

## C++ tests

### `make test` — phase stress test

Builds `test` from `main_phase_test.cpp` and runs it. The program:

1. Checks `inverse_move` (apply + inverse restores the cube) on random states.
2. Runs **500** random scrambles (length **15–40** moves).
3. For each solve, checks solution length (≤ **52** moves) and time (≤ **3000** ms).
4. Prints a progress bar and a summary (best/worst/avg time and move count).

```bash
make test
./test
```

`make fclean` removes the `test` binary.

### `make test_performance` — performance output sample

Builds `test_performance` from `test_performance.cpp`. It generates one random scramble (**20–40** moves), solves it, and prints **`performance_solution()`** (telemetry-style string from the solver).

```bash
make test_performance
./test_performance
```

`make fclean` removes the `test_performance` binary.

## Python visualizer (`make v`)

You do **not** need to create or activate a venv by hand. From the project root:

```bash
make v
```

This will:

1. Create `venv/` with `python3 -m venv` if it does not exist.
2. Run `pip install -r requirements.txt` when `requirements.txt` changes (tracked via `venv/.deps_installed`).
3. Run `venv/bin/python3 visualizer/main.py`.

To remove the virtual environment completely:

```bash
make clean-venv
```

### Manual venv (optional)

If you prefer not to use `make v`:

```bash
python3 -m venv venv
source venv/bin/activate   # Windows: venv\Scripts\activate
pip install -r requirements.txt
python3 visualizer/main.py
```

## Visualizer Usage

Main workflow:

1. Enter scramble moves in the scramble input.
2. Optionally set scramble length and click `Generate scramble`.
3. Click `Run ./rubik` to call the C++ solver.
4. Use `Load sequence`, `Play`, `Step`, `Back`, `Scramble only`, and `Solve only` for playback.

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

`No module named pygame` (or similar)

- Run `make v` so dependencies install into `venv`, or activate `venv` and run `pip install -r requirements.txt`.

`Invalid move: ...`

- Check notation and spacing in the move sequence.
