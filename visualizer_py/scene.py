"""3D scene renderer for the Rubik's cube visualizer."""

import math
import numpy as np
import pygame

try:
    from .moves import FACE_COLORS
except ImportError:
    from moves import FACE_COLORS


class CubeRenderer:
    """Simple cube renderer for visualization reference."""

    def __init__(self, width: int = 1200, height: int = 900):
        """Initialize the renderer.

        Args:
            width: Display width
            height: Display height
        """
        self.width = width
        self.height = height
        self.aspect = width / height if height > 0 else 1.0

        # Camera settings
        self.camera_pos = [7.5, 6.0, 7.5]
        self.camera_target = [0.0, 0.0, 0.0]
        self.camera_distance = np.linalg.norm(
            np.array(self.camera_pos) - np.array(self.camera_target)
        )
        self.fov_degrees = 50.0

        # Local cube geometry.
        half = 0.46
        self._local_vertices = np.array([
            [-half, -half, -half],
            [half, -half, -half],
            [half, half, -half],
            [-half, half, -half],
            [-half, -half, half],
            [half, -half, half],
            [half, half, half],
            [-half, half, half],
        ], dtype=float)

        # (indices, local normal, local face key)
        self._faces = [
            ((1, 2, 6, 5), np.array([1.0, 0.0, 0.0]), 'R'),
            ((0, 4, 7, 3), np.array([-1.0, 0.0, 0.0]), 'L'),
            ((3, 7, 6, 2), np.array([0.0, 1.0, 0.0]), 'U'),
            ((0, 1, 5, 4), np.array([0.0, -1.0, 0.0]), 'D'),
            ((4, 5, 6, 7), np.array([0.0, 0.0, 1.0]), 'F'),
            ((0, 3, 2, 1), np.array([0.0, 0.0, -1.0]), 'B'),
        ]

    def render(self, surface, cube_state, animation=None):
        """Render the cube onto a pygame surface.

        Args:
            surface: Target pygame surface (canvas area)
            cube_state: Cube instance containing cubie positions and rotations
            animation: Optional dict with {axis, layer, angle}
        """
        if surface is None or cube_state is None:
            return

        width, height = surface.get_size()
        view_basis = self._build_view_basis()
        right, up, forward = view_basis
        cam = np.array(self.camera_pos, dtype=float)

        fov = math.radians(self.fov_degrees)
        focal = (0.5 * min(width, height)) / math.tan(fov / 2.0)
        near = 0.1

        spacing = float(getattr(cube_state, 'spacing', 1.05))
        draw_items = []

        for cubie in cube_state.cubes:
            cubie_pos = np.array(cubie['pos'], dtype=float) * spacing
            rotation_state = cubie['rotation']
            if (
                isinstance(rotation_state, (list, tuple))
                and len(rotation_state) == 3
                and all(isinstance(v, (int, float)) for v in rotation_state)
            ):
                rx, ry, rz = rotation_state
                rot = self._rotation_matrix_xyz(rx, ry, rz)
            else:
                rot = np.array(rotation_state, dtype=float)

            extra_rot = None
            if animation is not None:
                axis = animation.get('axis')
                layer = int(animation.get('layer', 99))
                angle = float(animation.get('angle', 0.0))
                x, y, z = cubie['pos']
                if ((axis == 'x' and x == layer)
                        or (axis == 'y' and y == layer)
                        or (axis == 'z' and z == layer)):
                    extra_rot = self._rotation_matrix_axis(axis, angle)

            world_vertices = []
            cam_vertices = []

            for local_v in self._local_vertices:
                world_v = rot @ local_v + cubie_pos
                if extra_rot is not None:
                    world_v = extra_rot @ world_v
                world_vertices.append(world_v)

                rel = world_v - cam
                cx = np.dot(rel, right)
                cy = np.dot(rel, up)
                cz = np.dot(rel, forward)
                cam_vertices.append((cx, cy, cz))

            for indices, local_normal, local_face_key in self._faces:
                face_center = np.mean([world_vertices[i] for i in indices], axis=0)
                world_normal = rot @ local_normal
                if extra_rot is not None:
                    world_normal = extra_rot @ world_normal

                to_camera = cam - face_center
                if np.dot(world_normal, to_camera) <= 0.0:
                    continue

                projected = []
                depth_sum = 0.0
                clipped = False
                for i in indices:
                    cx, cy, cz = cam_vertices[i]
                    if cz <= near:
                        clipped = True
                        break

                    sx = (width * 0.5) + (cx * focal / cz)
                    sy = (height * 0.5) - (cy * focal / cz)
                    projected.append((sx, sy))
                    depth_sum += cz

                if clipped:
                    continue

                if local_face_key in cubie['sticker_faces']:
                    rgb = FACE_COLORS[cubie['sticker_faces'][local_face_key]]
                    color = (
                        int(max(0, min(255, rgb[0] * 255))),
                        int(max(0, min(255, rgb[1] * 255))),
                        int(max(0, min(255, rgb[2] * 255))),
                    )
                else:
                    color = (42, 42, 48)

                draw_items.append((depth_sum / 4.0, projected, color))

        draw_items.sort(key=lambda item: item[0], reverse=True)

        for _, poly, color in draw_items:
            pygame.draw.polygon(surface, color, poly)
            pygame.draw.polygon(surface, (12, 12, 16), poly, 1)

    def _build_view_basis(self):
        cam = np.array(self.camera_pos, dtype=float)
        target = np.array(self.camera_target, dtype=float)

        forward = target - cam
        forward_norm = np.linalg.norm(forward)
        if forward_norm < 1e-8:
            forward = np.array([0.0, 0.0, -1.0])
        else:
            forward = forward / forward_norm

        world_up = np.array([0.0, 1.0, 0.0])
        right = np.cross(forward, world_up)
        right_norm = np.linalg.norm(right)
        if right_norm < 1e-8:
            right = np.array([1.0, 0.0, 0.0])
        else:
            right = right / right_norm

        up = np.cross(right, forward)
        up_norm = np.linalg.norm(up)
        if up_norm < 1e-8:
            up = np.array([0.0, 1.0, 0.0])
        else:
            up = up / up_norm

        return right, up, forward

    def _rotation_matrix_xyz(self, rx: float, ry: float, rz: float):
        cx, sx = math.cos(rx), math.sin(rx)
        cy, sy = math.cos(ry), math.sin(ry)
        cz, sz = math.cos(rz), math.sin(rz)

        rx_m = np.array([
            [1.0, 0.0, 0.0],
            [0.0, cx, -sx],
            [0.0, sx, cx],
        ])

        ry_m = np.array([
            [cy, 0.0, sy],
            [0.0, 1.0, 0.0],
            [-sy, 0.0, cy],
        ])

        rz_m = np.array([
            [cz, -sz, 0.0],
            [sz, cz, 0.0],
            [0.0, 0.0, 1.0],
        ])

        return rz_m @ ry_m @ rx_m

    def _rotation_matrix_axis(self, axis: str, angle: float):
        c = math.cos(angle)
        s = math.sin(angle)

        if axis == 'x':
            return np.array([
                [1.0, 0.0, 0.0],
                [0.0, c, -s],
                [0.0, s, c],
            ])

        if axis == 'y':
            return np.array([
                [c, 0.0, s],
                [0.0, 1.0, 0.0],
                [-s, 0.0, c],
            ])

        return np.array([
            [c, -s, 0.0],
            [s, c, 0.0],
            [0.0, 0.0, 1.0],
        ])

    def resize(self, width: int, height: int):
        """Handle window resize.

        Args:
            width: New window width
            height: New window height
        """
        self.width = width
        self.height = height
        self.aspect = width / height if height > 0 else 1.0

    def rotate_camera(self, angle_x: float, angle_y: float):
        """Rotate camera around the cube.

        Args:
            angle_x: Rotation around x-axis
            angle_y: Rotation around y-axis
        """
        # Convert to spherical and apply rotation
        rel = np.array(self.camera_pos) - np.array(self.camera_target)
        distance = np.linalg.norm(rel)

        if distance > 0:
            theta = np.arctan2(rel[2], rel[0])
            phi = np.arccos(rel[1] / distance) if distance > 0 else 0

            theta += angle_y
            phi += angle_x
            phi = max(0.1, min(np.pi - 0.1, phi))

            self.camera_pos = np.array(self.camera_target) + np.array([
                distance * np.sin(phi) * np.cos(theta),
                distance * np.cos(phi),
                distance * np.sin(phi) * np.sin(theta)
            ]).tolist()

    def zoom_camera(self, delta: float):
        """Zoom camera in/out.

        Args:
            delta: Change in distance (positive = zoom out)
        """
        rel = np.array(self.camera_pos) - np.array(self.camera_target)
        distance = np.linalg.norm(rel)

        if distance > 0:
            direction = rel / distance
            new_distance = max(6.0, min(18.0, distance + delta))
            self.camera_pos = (
                np.array(self.camera_target) + direction * new_distance
            ).tolist()
            self.camera_distance = new_distance

    def reset_view(self):
        """Reset camera to default position."""
        self.camera_pos = [7.5, 6.0, 7.5]
        self.camera_target = [0.0, 0.0, 0.0]
        self.camera_distance = np.linalg.norm(
            np.array(self.camera_pos) - np.array(self.camera_target)
        )
