"""Rubik's cube move definitions and parsing."""

from typing import List, TypedDict


class MoveParams(TypedDict):
    axis: str
    layer: int
    direction: int
    turns: int

VALID_MOVES = {
    'R', 'L', 'U', 'D', 'F', 'B',
    "R'", "L'", "U'", "D'", "F'", "B'",
    'R2', 'L2', 'U2', 'D2', 'F2', 'B2',
}

MOVE_DEF = {
    # base_dir signs are chosen to match the solver's move convention
    # (so that e.g. "U" corresponds to the same rotation direction
    # as the C++ solver's `apply_move`). Flip signs if notation
    # appears inverted in the UI.
    'R': {'axis': 'x', 'layer': 1, 'base_dir': -1},
    'L': {'axis': 'x', 'layer': -1, 'base_dir': 1},
    'U': {'axis': 'y', 'layer': 1, 'base_dir': -1},
    'D': {'axis': 'y', 'layer': -1, 'base_dir': 1},
    'F': {'axis': 'z', 'layer': 1, 'base_dir': 1},
    'B': {'axis': 'z', 'layer': -1, 'base_dir': -1},
}

# Face colors (RGB)
FACE_COLORS = {
    'U': (0.90, 0.90, 0.90),  # White
    'D': (0.85, 0.70, 0.00),  # Yellow
    'F': (0.04, 0.48, 0.27),  # Green
    'B': (0.11, 0.24, 0.55),  # Blue
    'L': (0.82, 0.31, 0.00),  # Orange
    'R': (0.56, 0.12, 0.18),  # Red
}


def parse_moves(raw: str) -> List[str]:
    """Parse a move sequence string into a list of valid moves.

    Args:
        raw: Space-separated move sequence (e.g., "R U R' U'")

    Returns:
        List of valid moves

    Raises:
        ValueError: If invalid moves are found
    """
    tokens = str(raw or '').strip().split()
    tokens = [t for t in tokens if t]

    invalid = [t for t in tokens if t not in VALID_MOVES]
    if invalid:
        raise ValueError(f"Invalid move(s): {', '.join(invalid)}")

    return tokens


def invert_move(move: str) -> str:
    """Get the inverse of a move.

    Args:
        move: A valid move string

    Returns:
        The inverse move
    """
    if move.endswith('2'):
        return move
    if move.endswith("'"):
        return move[:-1]
    return f"{move}'"


def get_move_params(move: str) -> MoveParams:
    """ Get parameters for applying a move """
    base = move[0]
    turns = 2 if move.endswith('2') else 1
    base_def = MOVE_DEF[base]

    if move.endswith("'"):
        direction = -base_def['base_dir']
    else:
        direction = base_def['base_dir']

    return {
        'axis': base_def['axis'],
        'layer': base_def['layer'],
        'direction': direction,
        'turns': turns,
    }
