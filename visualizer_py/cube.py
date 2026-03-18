"""Rubik's cube state management."""

import copy

try:
    from .moves import get_move_params
except ImportError:
    from moves import get_move_params


class Cube:
    """Represents the state of a Rubik's cube in 3D space."""

    def __init__(self):
        """Initialize cube with 27 unit cubes (including center core)."""
        self.cubes = []
        self.spacing = 1.05

        # Create all 27 positions (-1, 0, 1 for each axis)
        for x in [-1, 0, 1]:
            for y in [-1, 0, 1]:
                for z in [-1, 0, 1]:
                    cube_data = {
                        'home_pos': (x, y, z),
                        'pos': (x, y, z),
                        'rotation': [0, 0, 0],  # Euler angles in radians
                        'sticker_faces': self._get_sticker_faces(x, y, z),
                    }
                    self.cubes.append(cube_data)

    def _get_sticker_faces(self, x: int, y: int, z: int) -> dict:
        """Determine which faces have stickers for a cubie at position (x, y, z).

        Returns:
            Dict mapping face keys ('U', 'D', 'F', 'B', 'L', 'R') to their colors.
            Center core has no stickers.
        """
        faces = {}
        if x == 1:
            faces['R'] = 'R'
        elif x == -1:
            faces['L'] = 'L'

        if y == 1:
            faces['U'] = 'U'
        elif y == -1:
            faces['D'] = 'D'

        if z == 1:
            faces['F'] = 'F'
        elif z == -1:
            faces['B'] = 'B'

        return faces

    def get_cubes_in_layer(self, axis: str, layer: int) -> list:
        """Get all cubies in a specific layer.

        Args:
            axis: 'x', 'y', or 'z'
            layer: -1, 0, or 1

        Returns:
            List of cubie dictionaries
        """
        result = []
        for cube in self.cubes:
            x, y, z = cube['pos']
            if axis == 'x' and x == layer:
                result.append(cube)
            elif axis == 'y' and y == layer:
                result.append(cube)
            elif axis == 'z' and z == layer:
                result.append(cube)
        return result

    def rotate_cubes_in_layer(self, axis: str, layer: int, angle: float):
        """Rotate cubies in a layer around an axis.

        Args:
            axis: 'x', 'y', or 'z'
            layer: -1, 0, or 1
            angle: Rotation angle in radians
        """
        cubes_to_rotate = self.get_cubes_in_layer(axis, layer)

        for cube in cubes_to_rotate:
            # Update position via rotation
            x, y, z = cube['pos']

            if axis == 'x':
                # Rotate around x-axis
                cos_a, sin_a = cos(angle), sin(angle)
                new_y = y * cos_a - z * sin_a
                new_z = y * sin_a + z * cos_a
                cube['pos'] = (x, new_y, new_z)
                cube['rotation'][0] += angle
            elif axis == 'y':
                # Rotate around y-axis
                cos_a, sin_a = cos(angle), sin(angle)
                new_x = x * cos_a + z * sin_a
                new_z = -x * sin_a + z * cos_a
                cube['pos'] = (new_x, y, new_z)
                cube['rotation'][1] += angle
            elif axis == 'z':
                # Rotate around z-axis
                cos_a, sin_a = cos(angle), sin(angle)
                new_x = x * cos_a - y * sin_a
                new_y = x * sin_a + y * cos_a
                cube['pos'] = (new_x, new_y, z)
                cube['rotation'][2] += angle

            # Normalize position (snap to grid)
            x, y, z = cube['pos']
            cube['pos'] = (self._snap_coord(x), self._snap_coord(y), self._snap_coord(z))

            # Normalize rotation
            cube['rotation'] = [self._snap_angle(r) for r in cube['rotation']]

    def _snap_coord(self, v: float) -> int:
        """Snap a coordinate to nearest integer (-1, 0, or 1)."""
        n = round(v)
        if n > 1:
            return 1
        if n < -1:
            return -1
        return n

    def _snap_angle(self, angle: float) -> float:
        """Snap an angle to nearest 90-degree increment."""
        import math
        quarter_pi = math.pi / 2
        return round(angle / quarter_pi) * quarter_pi

    def apply_move(self, move: str):
        """Apply a move to the cube (without animation).

        Args:
            move: Valid move string (e.g., 'R', "U'", 'F2')
        """
        import math
        params = get_move_params(move)

        # 90 degrees per quarter turn
        angle_per_turn = params['direction'] * (math.pi / 2)

        for _ in range(params['turns']):
            self.rotate_cubes_in_layer(
                params['axis'],
                params['layer'],
                angle_per_turn
            )

    def reset(self):
        """Reset cube to solved state."""
        for cube in self.cubes:
            cube['pos'] = cube['home_pos']
            cube['rotation'] = [0, 0, 0]

    def copy(self):
        """Return a deep copy of the cube state."""
        return copy.deepcopy(self)


# Import after class definition to avoid circular imports
from math import cos, sin
