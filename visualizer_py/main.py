"""Main Rubik's Cube visualizer application using pygame."""

import math
import os
import random
import subprocess
import time
from pathlib import Path
from typing import Callable, Dict, List, Optional, Tuple, TypedDict

import pygame

from cube import Cube
from moves import invert_move, parse_moves
from scene import CubeRenderer


class AnimationState(TypedDict):
    move: str
    axis: str
    layer: int
    target: float
    elapsed: float
    duration: float
    on_complete: Callable[[], None]


class CubeVisualizer:
    """Desktop Rubik's Cube visualizer with UI controls and playback."""

    MANUAL_MOVES = [
        "R", "R'", "R2", "L", "L'", "L2",
        "U", "U'", "U2", "D", "D'", "D2",
        "F", "F'", "F2", "B", "B'", "B2",
    ]

    def __init__(self, width: int = 1400, height: int = 900):
        pygame.init()

        self.width = width
        self.height = height
        self.running = True
        self.clock = pygame.time.Clock()
        self.fps = 60

        self.screen = pygame.display.set_mode((width, height), pygame.RESIZABLE)
        pygame.display.set_caption("Rubik's Cube Visualizer")

        self.renderer = CubeRenderer(width, height)
        self.cube = Cube()

        self.scramble_text = "R U R' U'"
        self.solve_text = "U R U' R'"
        self.active_input: Optional[str] = None

        self.error_text = ""
        self.status_text = "Ready"
        self.last_exec_time_ms: Optional[float] = None
        self.last_exec_input = ""
        self.scramble_len_text = "20"

        self.sequence: List[str] = []
        self.scramble_moves: List[str] = []
        self.solve_moves: List[str] = []
        self.seq_index = 0
        self.applied_moves: List[str] = []

        self.playing = False
        self.batch_queue: List[Tuple[str, Optional[int]]] = []

        self.animation_duration_ms = 240
        self.current_move = ""
        self.anim: Optional[AnimationState] = None

        self.mouse_dragging = False
        self.mouse_last_pos: Optional[Tuple[int, int]] = None
        self.drag_started_on_canvas = False
        self.dragging_slider = False

        self.panel_width = 380
        self.rects: Dict[str, pygame.Rect] = {}
        self.manual_buttons: List[Tuple[pygame.Rect, str]] = []
        self._setup_ui_rects()

        self._fonts = {
            "title": pygame.font.Font(None, 28),
            "label": pygame.font.Font(None, 18),
            "body": pygame.font.Font(None, 17),
            "mono": pygame.font.SysFont("couriernew,consolas,monospace", 16),
            "mono_small": pygame.font.SysFont("couriernew,consolas,monospace", 14),
        }

        self.load_sequence(reset_cube=True)

    @property
    def busy(self) -> bool:
        return self.anim is not None

    def _setup_ui_rects(self):
        margin = 12
        gap = 8
        btn_h = 34

        self.panel_rect = pygame.Rect(0, 0, self.panel_width, self.height)
        self.canvas_rect = pygame.Rect(self.panel_width, 0, self.width - self.panel_width, self.height)

        y = margin + 52
        input_h = 48

        self.rects["scramble_input"] = pygame.Rect(margin, y, self.panel_width - margin * 2, input_h)
        y += input_h + gap + 26
        self.rects["solve_input"] = pygame.Rect(margin, y, self.panel_width - margin * 2, input_h)
        y += input_h + gap + 10
        self.rects["scramble_len_input"] = pygame.Rect(margin, y, 116, btn_h)
        self.rects["generate_scramble"] = pygame.Rect(margin + 116 + gap, y, self.panel_width - margin * 2 - 116 - gap, btn_h)
        y += btn_h + gap

        half_w = (self.panel_width - margin * 3) // 2
        self.rects["load"] = pygame.Rect(margin, y, half_w, btn_h)
        self.rects["reset_cube"] = pygame.Rect(margin + half_w + margin, y, half_w, btn_h)
        y += btn_h + gap

        self.rects["reset_view"] = pygame.Rect(margin, y, self.panel_width - margin * 2, btn_h)
        y += btn_h + 16

        self.rects["back"] = pygame.Rect(margin, y, half_w, btn_h)
        self.rects["step"] = pygame.Rect(margin + half_w + margin, y, half_w, btn_h)
        y += btn_h + gap

        self.rects["play"] = pygame.Rect(margin, y, half_w, btn_h)
        self.rects["pause"] = pygame.Rect(margin + half_w + margin, y, half_w, btn_h)
        y += btn_h + gap

        self.rects["scramble_only"] = pygame.Rect(margin, y, half_w, btn_h)
        self.rects["solve_only"] = pygame.Rect(margin + half_w + margin, y, half_w, btn_h)
        y += btn_h + gap

        self.rects["run_solver"] = pygame.Rect(margin, y, self.panel_width - margin * 2, btn_h)
        y += btn_h + 22

        self.rects["speed_track"] = pygame.Rect(margin, y + 18, self.panel_width - margin * 2 - 90, 10)
        self.rects["speed_label"] = pygame.Rect(self.rects["speed_track"].right + 10, y + 7, 82, 22)
        y += 60

        self.manual_buttons = []
        cols = 6
        manual_w = (self.panel_width - margin * 2 - (cols - 1) * gap) // cols
        manual_h = 28
        for idx, move in enumerate(self.MANUAL_MOVES):
            row = idx // cols
            col = idx % cols
            rx = margin + col * (manual_w + gap)
            ry = y + row * (manual_h + gap)
            self.manual_buttons.append((pygame.Rect(rx, ry, manual_w, manual_h), move))
        y += (manual_h + gap) * 3 + 8

        self.rects["status"] = pygame.Rect(margin, y, self.panel_width - margin * 2, self.height - y - margin)

    def handle_events(self):
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                self.running = False
            elif event.type == pygame.VIDEORESIZE:
                self._handle_resize(event)
            elif event.type == pygame.MOUSEBUTTONDOWN:
                self._handle_mouse_down(event)
            elif event.type == pygame.MOUSEBUTTONUP:
                self._handle_mouse_up(event)
            elif event.type == pygame.MOUSEMOTION:
                self._handle_mouse_motion(event)
            elif event.type == pygame.MOUSEWHEEL:
                self._handle_mouse_wheel(event)
            elif event.type == pygame.KEYDOWN:
                self._handle_keydown(event)

    def _handle_resize(self, event):
        self.width = max(960, event.w)
        self.height = max(620, event.h)
        self.screen = pygame.display.set_mode((self.width, self.height), pygame.RESIZABLE)
        self.renderer.resize(self.width, self.height)
        self._setup_ui_rects()

    def _handle_mouse_down(self, event):
        if event.button != 1:
            return

        pos = pygame.mouse.get_pos()
        self.mouse_dragging = True
        self.mouse_last_pos = pos
        self.drag_started_on_canvas = self.canvas_rect.collidepoint(pos)

        if self.panel_rect.collidepoint(pos):
            self._handle_panel_click(pos)
        elif self.canvas_rect.collidepoint(pos):
            self.active_input = None

        speed_track = self.rects["speed_track"]
        if speed_track.inflate(0, 12).collidepoint(pos):
            self.dragging_slider = True
            self._set_speed_from_mouse(pos[0])

    def _handle_mouse_up(self, event):
        if event.button == 1:
            self.mouse_dragging = False
            self.mouse_last_pos = None
            self.drag_started_on_canvas = False
            self.dragging_slider = False

    def _handle_mouse_motion(self, _event):
        pos = pygame.mouse.get_pos()

        if self.dragging_slider:
            self._set_speed_from_mouse(pos[0])

        if not self.mouse_dragging or not self.drag_started_on_canvas or self.mouse_last_pos is None:
            return
        if not self.canvas_rect.collidepoint(pos):
            self.mouse_last_pos = pos
            return

        dx = pos[0] - self.mouse_last_pos[0]
        dy = pos[1] - self.mouse_last_pos[1]
        self.renderer.rotate_camera(dy * 0.005, dx * 0.005)
        self.mouse_last_pos = pos

    def _handle_mouse_wheel(self, event):
        self.renderer.zoom_camera(-event.y * 0.5)

    def _handle_keydown(self, event):
        if event.key == pygame.K_ESCAPE:
            self.running = False
            return

        if self.active_input is not None:
            self._handle_text_input_key(event)
            return

        if event.key == pygame.K_SPACE:
            self.playing = not self.playing
            self.batch_queue = []
            self.status_text = "Playing" if self.playing else "Paused"
        elif event.key == pygame.K_RIGHT:
            self.pause_all()
            self.step_forward()
        elif event.key == pygame.K_LEFT or event.key == pygame.K_BACKSPACE:
            self.pause_all()
            self.step_back()
        elif event.key == pygame.K_r:
            self.reset_cube()
        elif event.key == pygame.K_v:
            self.renderer.reset_view()
            self.status_text = "View reset"

    def _handle_text_input_key(self, event):
        key = self.active_input
        if key is None:
            return

        if key == "scramble":
            text = self.scramble_text
        elif key == "solve":
            text = self.solve_text
        else:
            text = self.scramble_len_text

        if event.key == pygame.K_RETURN:
            self.active_input = None
            return
        if event.key == pygame.K_TAB:
            self.active_input = "solve" if key == "scramble" else "scramble"
            return
        if event.key == pygame.K_BACKSPACE:
            text = text[:-1]
        elif event.unicode and event.unicode.isprintable() and event.unicode not in "\r\n\t":
            if key == "scramble_len":
                if event.unicode.isdigit() and len(text) < 3:
                    text += event.unicode
            else:
                text += event.unicode

        if key == "scramble":
            self.scramble_text = text
        elif key == "solve":
            self.solve_text = text
        else:
            self.scramble_len_text = text

    def _set_speed_from_mouse(self, mouse_x: int):
        track = self.rects["speed_track"]
        t = (mouse_x - track.left) / max(1, track.width)
        t = max(0.0, min(1.0, t))
        min_ms, max_ms = 80, 900
        value = int(round(min_ms + (max_ms - min_ms) * t))
        self.animation_duration_ms = value - (value % 10)

    def _can_interact(self) -> bool:
        return not self.busy

    def _handle_panel_click(self, pos):
        if self.rects["scramble_input"].collidepoint(pos):
            self.active_input = "scramble"
            return
        if self.rects["solve_input"].collidepoint(pos):
            self.active_input = "solve"
            return
        if self.rects["scramble_len_input"].collidepoint(pos):
            self.active_input = "scramble_len"
            return

        if self.rects["generate_scramble"].collidepoint(pos):
            self.pause_all()
            if self._can_interact():
                self._generate_scramble_from_length()
            return

        if self.rects["load"].collidepoint(pos):
            self.pause_all()
            self.load_sequence(reset_cube=False)
            return
        if self.rects["reset_cube"].collidepoint(pos):
            self.reset_cube()
            return
        if self.rects["reset_view"].collidepoint(pos):
            self.pause_all()
            self.renderer.reset_view()
            self.status_text = "View reset"
            return

        if self.rects["back"].collidepoint(pos):
            self.pause_all()
            self.step_back()
            return
        if self.rects["step"].collidepoint(pos):
            self.pause_all()
            self.step_forward()
            return
        if self.rects["play"].collidepoint(pos):
            if self.sequence:
                self.batch_queue = []
                self.playing = True
                self.status_text = "Playing"
            return
        if self.rects["pause"].collidepoint(pos):
            self.pause_all()
            self.status_text = "Paused"
            return

        if self.rects["scramble_only"].collidepoint(pos):
            self.pause_all()
            if self.load_sequence(reset_cube=True):
                self.batch_queue = [(m, i) for i, m in enumerate(self.scramble_moves)]
                self.status_text = "Running scramble"
            return

        if self.rects["solve_only"].collidepoint(pos):
            self.pause_all()
            if self.solve_moves:
                start = len(self.scramble_moves)
                self.seq_index = max(self.seq_index, start)
                self.batch_queue = [(m, start + i) for i, m in enumerate(self.solve_moves)]
                self.status_text = "Running solve"
            return

        if self.rects["run_solver"].collidepoint(pos):
            self.pause_all()
            if self._can_interact():
                self._run_solver_executable()
            return

        if not self._can_interact():
            return

        for rect, move in self.manual_buttons:
            if rect.collidepoint(pos):
                self.pause_all()
                self._start_move_animation(move, self._after_manual_move(move))
                return

    def pause_all(self):
        self.playing = False
        self.batch_queue = []

    def _generate_scramble_from_length(self):
        raw_len = self.scramble_len_text.strip() or "0"
        if not raw_len.isdigit():
            self.error_text = "Scramble length must be a positive integer"
            self.status_text = "Invalid scramble length"
            return

        length = int(raw_len)
        if length <= 0:
            self.error_text = "Scramble length must be greater than zero"
            self.status_text = "Invalid scramble length"
            return

        faces = ["R", "L", "U", "D", "F", "B"]
        suffixes = ["", "'", "2"]
        moves: List[str] = []
        prev_face = ""

        for _ in range(length):
            candidates = [face for face in faces if face != prev_face]
            face = random.choice(candidates)
            move = face + random.choice(suffixes)
            moves.append(move)
            prev_face = face

        self.scramble_text = " ".join(moves)
        self.solve_text = ""
        self.error_text = ""
        self.last_exec_input = self.scramble_text

        self.load_sequence(reset_cube=True)
        self.status_text = f"Generated scramble ({length} moves)"

    def _run_solver_executable(self):
        move_text = self.scramble_text.strip()
        self.error_text = ""

        try:
            parse_moves(move_text)
        except ValueError as exc:
            self.error_text = str(exc)
            self.status_text = "Invalid scramble sequence"
            return

        solver_path = Path(__file__).resolve().parent.parent / "rubik"
        if not solver_path.exists():
            self.status_text = "Solver executable not found"
            self.error_text = f"Missing: {solver_path}"
            return
        if not os.access(solver_path, os.X_OK):
            self.status_text = "Solver is not executable"
            self.error_text = f"Run: chmod +x {solver_path}"
            return

        start = time.perf_counter()
        result = subprocess.run(
            [str(solver_path), move_text],
            capture_output=True,
            text=True,
            check=False,
        )
        elapsed_ms = (time.perf_counter() - start) * 1000.0
        self.last_exec_time_ms = elapsed_ms
        self.last_exec_input = move_text

        if result.returncode != 0:
            err = (result.stderr or result.stdout or "Solver failed").strip()
            self.error_text = err[:160]
            self.status_text = f"Solver failed ({elapsed_ms:.2f} ms)"
            return

        solved_moves = [line.strip() for line in result.stdout.splitlines() if line.strip()]
        self.solve_text = " ".join(solved_moves)
        self.load_sequence(reset_cube=True)
        self.status_text = f"Solver finished in {elapsed_ms:.2f} ms"

    def load_sequence(self, reset_cube: bool) -> bool:
        self.error_text = ""
        try:
            self.scramble_moves = parse_moves(self.scramble_text)
            self.solve_moves = parse_moves(self.solve_text)
        except ValueError as exc:
            self.error_text = str(exc)
            self.sequence = []
            self.scramble_moves = []
            self.solve_moves = []
            self.seq_index = 0
            self.status_text = "Invalid sequence"
            return False

        self.sequence = self.scramble_moves + self.solve_moves
        self.seq_index = 0
        self.applied_moves = []
        if reset_cube:
            self.cube.reset()
        self.status_text = f"Loaded: {len(self.scramble_moves)} + {len(self.solve_moves)}"
        return True

    def reset_cube(self):
        self.pause_all()
        self.anim = None
        self.current_move = ""
        self.cube.reset()
        self.applied_moves = []
        self.seq_index = 0
        self.error_text = ""
        self.status_text = "Reset to solved"

    def _start_move_animation(self, move: str, on_complete: Callable[[], None]):
        if self.busy:
            return

        base = move[0]
        turns = 2 if move.endswith("2") else 1
        duration = max(80, int(self.animation_duration_ms)) * turns

        mapping = {
            "R": {"axis": "x", "layer": 1, "base_dir": 1},
            "L": {"axis": "x", "layer": -1, "base_dir": -1},
            "U": {"axis": "y", "layer": 1, "base_dir": 1},
            "D": {"axis": "y", "layer": -1, "base_dir": -1},
            "F": {"axis": "z", "layer": 1, "base_dir": -1},
            "B": {"axis": "z", "layer": -1, "base_dir": 1},
        }
        m = mapping[base]
        direction = -m["base_dir"] if move.endswith("'") else m["base_dir"]
        target_angle = (math.pi / 2) * direction * turns

        self.current_move = move
        self.anim = {
            "move": move,
            "axis": m["axis"],
            "layer": m["layer"],
            "target": target_angle,
            "elapsed": 0.0,
            "duration": duration / 1000.0,
            "on_complete": on_complete,
        }

    def _after_manual_move(self, move: str) -> Callable[[], None]:
        def done():
            self.applied_moves.append(move)
            self.status_text = f"Applied: {move}"
        return done

    def step_forward(self):
        if self.busy or self.seq_index >= len(self.sequence):
            return
        move = self.sequence[self.seq_index]

        def done():
            self.applied_moves.append(move)
            self.seq_index += 1
            self.status_text = f"Applied: {move}"

        self._start_move_animation(move, done)

    def step_back(self):
        if self.busy or not self.applied_moves:
            return

        move = self.applied_moves.pop()
        inverse = invert_move(move)

        def done():
            if self.seq_index > 0 and self.sequence and self.sequence[self.seq_index - 1] == move:
                self.seq_index -= 1
            self.status_text = f"Undid: {move}"

        self._start_move_animation(inverse, done)

    def update(self, delta_time: float):
        if self.anim is not None:
            self.anim["elapsed"] = float(self.anim["elapsed"]) + delta_time
            progress = min(1.0, float(self.anim["elapsed"]) / max(1e-6, float(self.anim["duration"])))
            if progress >= 1.0:
                move = str(self.anim["move"])
                callback = self.anim["on_complete"]
                self.cube.apply_move(move)
                self.anim = None
                self.current_move = ""
                callback()
            return

        if self.batch_queue:
            move, idx = self.batch_queue.pop(0)

            def done():
                self.applied_moves.append(move)
                if idx is not None:
                    self.seq_index = max(self.seq_index, idx + 1)
                self.status_text = f"Applied: {move}"

            self._start_move_animation(move, done)
            return

        if self.playing:
            if self.seq_index >= len(self.sequence):
                self.playing = False
                self.status_text = "Done"
            else:
                self.step_forward()

    def _draw_button(self, surface, rect, text, enabled=True, accent=False):
        mouse = pygame.mouse.get_pos()
        hovered = rect.collidepoint(mouse)

        if not enabled:
            fill = (46, 46, 60)
            border = (72, 72, 90)
            text_color = (120, 120, 135)
        elif accent:
            fill = (54, 79, 64) if hovered else (46, 67, 56)
            border = (110, 170, 128)
            text_color = (220, 240, 224)
        else:
            fill = (72, 74, 96) if hovered else (64, 66, 88)
            border = (106, 110, 145)
            text_color = (224, 227, 238)

        pygame.draw.rect(surface, fill, rect, border_radius=10)
        pygame.draw.rect(surface, border, rect, 1, border_radius=10)
        label = self._fonts["body"].render(text, True, text_color)
        surface.blit(label, (rect.centerx - label.get_width() // 2, rect.centery - label.get_height() // 2))

    def _draw_input(self, surface, label, key, rect, text):
        label_surface = self._fonts["label"].render(label, True, (178, 184, 205))
        surface.blit(label_surface, (rect.left, rect.top - 18))

        active = self.active_input == key
        fill = (26, 28, 39) if not active else (34, 37, 52)
        border = (88, 95, 132) if not active else (122, 138, 210)
        pygame.draw.rect(surface, fill, rect, border_radius=10)
        pygame.draw.rect(surface, border, rect, 1, border_radius=10)

        content = text
        if active and pygame.time.get_ticks() % 900 < 500:
            content += "_"

        clip = surface.get_clip()
        surface.set_clip(rect.inflate(-10, -10))
        txt = self._fonts["mono"].render(content, True, (232, 235, 245))
        surface.blit(txt, (rect.left + 8, rect.centery - txt.get_height() // 2))
        surface.set_clip(clip)

    def _draw_speed_slider(self, surface):
        label = self._fonts["label"].render("Move speed", True, (178, 184, 205))
        track = self.rects["speed_track"]
        surface.blit(label, (track.left, track.top - 20))

        pygame.draw.rect(surface, (42, 45, 58), track, border_radius=8)
        pygame.draw.rect(surface, (80, 86, 112), track, 1, border_radius=8)

        min_ms, max_ms = 80, 900
        t = (self.animation_duration_ms - min_ms) / float(max_ms - min_ms)
        t = max(0.0, min(1.0, t))
        knob_x = int(track.left + t * track.width)

        fill_track = pygame.Rect(track.left, track.top, max(1, knob_x - track.left), track.height)
        pygame.draw.rect(surface, (82, 146, 112), fill_track, border_radius=8)

        pygame.draw.circle(surface, (230, 236, 248), (knob_x, track.centery), 7)
        pygame.draw.circle(surface, (100, 108, 132), (knob_x, track.centery), 7, 1)

        speed_text = self._fonts["mono"].render(f"{self.animation_duration_ms} ms", True, (214, 220, 236))
        speed_rect = self.rects["speed_label"]
        surface.blit(speed_text, (speed_rect.right - speed_text.get_width(), speed_rect.top))

    def _draw_manual_grid(self, surface):
        if not self.manual_buttons:
            return

        first_rect = self.manual_buttons[0][0]
        label = self._fonts["label"].render("Manual move", True, (178, 184, 205))
        surface.blit(label, (first_rect.left, first_rect.top - 20))

        for rect, move in self.manual_buttons:
            self._draw_button(surface, rect, move, enabled=not self.busy)

    def _draw_status(self, surface):
        status_rect = self.rects["status"]

        pos = self._fonts["mono"].render(f"{self.seq_index} / {len(self.sequence)}", True, (216, 224, 243))
        surface.blit(pos, (status_rect.right - pos.get_width(), status_rect.top))

        if self.last_exec_time_ms is not None:
            elapsed = self._fonts["mono_small"].render(
                f"solver: {self.last_exec_time_ms:.2f} ms",
                True,
                (176, 212, 236),
            )
            surface.blit(elapsed, (status_rect.left, status_rect.top + 18))

            moves = self._fonts["mono_small"].render(
                f"moves: {len(self.solve_moves)}",
                True,
                (176, 212, 236),
            )
            surface.blit(moves, (status_rect.left, status_rect.top + 34))

        status_line = self.status_text or "Ready"
        status_surf = self._fonts["mono"].render(status_line, True, (150, 220, 170))
        surface.blit(status_surf, (status_rect.left, status_rect.top + 56))

        if self.error_text:
            err = self._fonts["mono"].render(self.error_text[:80], True, (238, 122, 122))
            surface.blit(err, (status_rect.left, status_rect.top + 78))

    def _draw_move_barrel(self):
        if self.canvas_rect.width <= 0 or self.canvas_rect.height <= 0:
            return

        center_x = self.canvas_rect.left + self.canvas_rect.width // 2
        top_y = self.canvas_rect.top + 16
        depth = 5

        for offset in range(-depth, depth + 1):
            idx = self.seq_index + offset
            text = self.sequence[idx] if 0 <= idx < len(self.sequence) else "-"
            dist = abs(offset)
            alpha = max(35, 255 - dist * 40)
            color = (210, 220, 236, alpha)

            text_surf = self._fonts["mono"].render(text, True, (color[0], color[1], color[2]))
            x = center_x + offset * 30 - text_surf.get_width() // 2
            y = top_y + dist * 4
            self.screen.blit(text_surf, (x, y))

            if offset == 0:
                underline = pygame.Rect(x - 2, y + text_surf.get_height() + 2, text_surf.get_width() + 4, 2)
                pygame.draw.rect(self.screen, (194, 204, 230), underline)

    def _draw_move_pill(self):
        text = self.current_move if self.current_move else "-"
        body = self._fonts["mono"].render(f"Move: {text}", True, (228, 232, 244))
        pad_x, pad_y = 10, 7
        rect = pygame.Rect(
            self.canvas_rect.right - body.get_width() - pad_x * 2 - 16,
            self.canvas_rect.bottom - body.get_height() - pad_y * 2 - 16,
            body.get_width() + pad_x * 2,
            body.get_height() + pad_y * 2,
        )
        pygame.draw.rect(self.screen, (20, 24, 34), rect, border_radius=14)
        pygame.draw.rect(self.screen, (90, 98, 130), rect, 1, border_radius=14)
        self.screen.blit(body, (rect.left + pad_x, rect.top + pad_y))

    def render(self):
        self.screen.fill((14, 16, 24))

        panel = pygame.Surface((self.panel_rect.width, self.panel_rect.height), pygame.SRCALPHA)
        panel.fill((22, 25, 36))

        title = self._fonts["title"].render("Rubik's Cube Visualizer", True, (230, 236, 248))
        subtitle = self._fonts["body"].render("Scramble + solve controls with 3D playback", True, (162, 168, 191))
        panel.blit(title, (12, 12))
        panel.blit(subtitle, (12, 36))

        self._draw_input(panel, "Scramble algorithm", "scramble", self.rects["scramble_input"], self.scramble_text)
        self._draw_input(panel, "Solve algorithm", "solve", self.rects["solve_input"], self.solve_text)
        self._draw_input(panel, "N", "scramble_len", self.rects["scramble_len_input"], self.scramble_len_text)
        self._draw_button(
            panel,
            self.rects["generate_scramble"],
            "Generate scramble",
            enabled=not self.busy,
        )

        self._draw_button(panel, self.rects["load"], "Load sequence", enabled=not self.busy)
        self._draw_button(panel, self.rects["reset_cube"], "Reset cube", enabled=not self.busy)
        self._draw_button(panel, self.rects["reset_view"], "Reset view", enabled=not self.busy)

        self._draw_button(panel, self.rects["back"], "Back", enabled=(not self.busy and bool(self.applied_moves)))
        self._draw_button(panel, self.rects["step"], "Step", enabled=(not self.busy and self.seq_index < len(self.sequence)))
        self._draw_button(panel, self.rects["play"], "Play", enabled=(not self.busy and bool(self.sequence)), accent=True)
        self._draw_button(panel, self.rects["pause"], "Pause", enabled=(not self.busy and self.playing))
        self._draw_button(
            panel,
            self.rects["scramble_only"],
            "Scramble only",
            enabled=(not self.busy and bool(self.scramble_moves)),
        )
        self._draw_button(
            panel,
            self.rects["solve_only"],
            "Solve only",
            enabled=(not self.busy and bool(self.solve_moves)),
        )
        self._draw_button(
            panel,
            self.rects["run_solver"],
            "Run ./rubik",
            enabled=(not self.busy and bool(self.scramble_text.strip())),
            accent=True,
        )

        self._draw_speed_slider(panel)
        self._draw_manual_grid(panel)
        self._draw_status(panel)

        self.screen.blit(panel, self.panel_rect.topleft)
        pygame.draw.rect(self.screen, (88, 95, 132), self.panel_rect, 1)

        canvas_surface = pygame.Surface((self.canvas_rect.width, self.canvas_rect.height))
        canvas_surface.fill((24, 28, 40))

        animation_payload = None
        if self.anim is not None:
            t = min(1.0, float(self.anim["elapsed"]) / max(1e-6, float(self.anim["duration"])))
            eased = 4 * t * t * t if t < 0.5 else 1 - ((-2 * t + 2) ** 3) / 2
            animation_payload = {
                "axis": str(self.anim["axis"]),
                "layer": int(self.anim["layer"]),
                "angle": float(self.anim["target"]) * eased,
            }

        self.renderer.render(canvas_surface, self.cube, animation_payload)
        self.screen.blit(canvas_surface, self.canvas_rect.topleft)

        pygame.draw.rect(self.screen, (88, 95, 132), self.canvas_rect, 1)
        self._draw_move_barrel()
        self._draw_move_pill()

        pygame.display.flip()

    def run(self):
        while self.running:
            dt = self.clock.tick(self.fps) / 1000.0
            self.handle_events()
            self.update(dt)
            self.render()
        pygame.quit()


def main():
    app = CubeVisualizer(width=1400, height=900)
    app.run()


if __name__ == "__main__":
    main()
