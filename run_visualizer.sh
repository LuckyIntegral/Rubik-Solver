#!/bin/bash
# Launch script for the Rubik's Cube Visualizer

# Get the directory where this script is located
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

# Activate virtual environment
source "$SCRIPT_DIR/venv/bin/activate"

# Run the visualizer
python "$SCRIPT_DIR/visualizer_py/main.py"
