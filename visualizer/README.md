# Rubik's Cube Visualizer - Python Edition

A desktop Rubik visualizer that mirrors the JS `visualizer/` functionality in Python with `pygame` + `numpy`.

## Features

- **3D Cube Visualization**: Colored 3D cubies with camera orbit and zoom
- **Algorithm Execution**: Scramble + solve sequence loading and playback
- **Interactive Controls**:
  - Drag to rotate the cube view
  - Scroll to zoom in/out
  - Step through moves forward and backward
  - Automatic playback of sequences
  - Scramble-only and solve-only execution
  - Manual move grid (`R/L/U/D/F/B`, prime, double)
  - Animated quarter-turn transitions with adjustable speed
  - Current move pill and sequence position HUD
  - Move barrel preview for surrounding moves
- **Keyboard Shortcuts**:
  - `LEFT/RIGHT` arrows: Step back/forward
  - `SPACE`: Play/Pause
  - `R`: Reset cube
  - `V`: Reset camera view
  - `ESC`: Exit application

## Installation

### Prerequisites

- Python 3.7+
- Virtual environment (recommended)

### Setup

1. **Activate the virtual environment**:
   ```bash
   source venv/bin/activate
   ```

2. **Install dependencies**:
   ```bash
   pip install -r requirements.txt
   ```

## Running the Visualizer

### Option 1: Run as a module (recommended)
```bash
python -m visualizer_py
```

### Option 2: Run the main script
```bash
cd visualizer_py
python main.py
```

### Option 3: Use the launch script
```bash
./run_visualizer.sh
```

## Usage

### Input Algorithms

1. **Scramble**: Enter a move sequence to scramble the cube (e.g., `R U R' U'`)
2. **Solve**: Enter the solution sequence to solve the scrambled cube
3. Click **Load** to parse and load both sequences

### Supported Moves

The visualizer supports standard Rubik's Cube notation:

- **Faces**: `U`, `D`, `F`, `B`, `L`, `R` (Up, Down, Front, Back, Left, Right)
- **Modifiers**:
  - Prime (counterclockwise): `'` suffix (e.g., `R'`)
  - Double turn (180°): `2` suffix (e.g., `R2`)

### Examples

```
R U R' U'          - Basic R-U-R'-U' sequence
F R U' R' F'       - Sexy move
M E S M' E' S'     - Slice moves
2R 2U 2R' 2U'      - Wide turns (note: currently not supported, use standard notation)
```

### Controls

**Mouse**:
- Left-click and drag: Rotate the cube
- Scroll wheel: Zoom in/out

**Buttons**:
- **Load sequence**: Parse scramble + solve text
- **Reset Cube**: Return cube to solved state
- **Reset View**: Reset camera to default position
- **Back**: Undo the last move
- **Step**: Execute the next move
- **Play**: Automatically play through the sequence
- **Pause**: Stop playback/batch execution
- **Scramble only**: Reset and run only scramble moves
- **Solve only**: Run only solve moves from current state

**Keyboard**:
- `←` / `→`: Step back/forward
- `SPACE`: Toggle playback
- `R`: Reset cube
- `V`: Reset camera
- `ESC`: Quit application

## Project Structure

```
visualizer_py/
├── main.py          # Main application and UI
├── cube.py          # Cube state management
├── moves.py         # Move definitions and parsing
├── scene.py         # Rendering (camera controls, etc.)
├── __init__.py      # Package initialization
└── __main__.py      # Module entry point

requirements.txt     # Python dependencies
```

## Dependencies

- **pygame** (2.5.2): Window management and UI rendering
- **PyOpenGL** (3.1.7): OpenGL bindings (for future 3D rendering)
- **PyGLM** (2.7.1): Matrix and vector math
- **numpy** (1.24.3): Numerical computations

## Features Implemented

✅ Move parsing and validation
✅ Cube state management
✅ Move execution and inversion
✅ Animated step-by-step playback
✅ Camera controls (rotation and zoom)
✅ Interactive UI with buttons, text input, and keyboard shortcuts
✅ Manual move grid
✅ Scramble-only and solve-only actions
✅ Move speed slider
✅ Move barrel and active move indicators
✅ Status display and error reporting

## Future Enhancements

- [ ] OpenGL-backed renderer for higher performance
- [ ] Support for wide turns (e.g., `Rw`, `2R`)
- [ ] Move undo/redo history
- [ ] Sequence editing with syntax highlighting
- [ ] Cube state import/export
- [ ] Performance profiling for complex sequences
- [ ] Customizable color schemes

## Troubleshooting

### "No module named 'pygame'"
Make sure you've activated the virtual environment and installed dependencies:
```bash
source venv/bin/activate
pip install -r requirements.txt
```

### "Invalid move" error
Check that your move notation is correct. Supported formats:
- Single moves: `R`, `U`, `F`, etc.
- Prime moves: `R'`, `U'`, etc.
- Double moves: `R2`, `U2`, etc.
- Separate moves with spaces: `R U R' U'`

### Performance issues
- Reduce the window size
- Close other applications
- Check that no other instances are running

## Credits

This is a Python port of the JavaScript Rubik's Cube Visualizer, adapted for desktop use with Pygame.

## License

MIT License - Feel free to use and modify!
