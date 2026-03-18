#!/usr/bin/env python3
"""
Rubik's Cube Visualizer
A Python-based 3D visualization tool for Rubik's cube algorithms.

Usage:
    python -m visualizer_py

Or:
    cd visualizer_py && python main.py
"""

from .main import CubeVisualizer

if __name__ == "__main__":
    app = CubeVisualizer(width=1400, height=900)
    app.run()
