"""Main Rubik's Cube visualizer application using pygame."""

from collections import deque
import math
import os
import random
import subprocess
import time
from pathlib import Path
from typing import Callable, Deque, Dict, List, Optional, Tuple, TypedDict

import pygame

from cube import Cube
from moves import get_move_params, invert_move, parse_moves
from scene import CubeRenderer
import ui


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

    ALGO_OPTIONS = ("algoA", "algoB")
    SCRAMBLE_FACES = ("R", "L", "U", "D", "F", "B")
    SCRAMBLE_SUFFIXES = ("", "'", "2")
    SPEED_MIN_MS = 80
    SPEED_MAX_MS = 900
    SPEED_STEP_MS = 10
    ROTATE_SENSITIVITY = 0.005
    WHEEL_ZOOM_STEP = 0.5

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
        self.algo_options = list(self.ALGO_OPTIONS)
        self.selected_algo = self.algo_options[0]
        self.algo_dropdown_open = False

        self.sequence: List[str] = []
        self.scramble_moves: List[str] = []
        self.solve_moves: List[str] = []
        self.seq_index = 0
        self.applied_moves: List[str] = []

        self.playing = False
        self.batch_queue: Deque[Tuple[str, Optional[int]]] = deque()

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
        ui.setup_ui_rects(self)

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
        self.renderer.rotate_camera(
            -dy * self.ROTATE_SENSITIVITY,
            dx * self.ROTATE_SENSITIVITY,
        )
        self.mouse_last_pos = pos

    def _handle_mouse_wheel(self, event):
        self.renderer.zoom_camera(-event.y * self.WHEEL_ZOOM_STEP)

    def _handle_keydown(self, event):
        if event.key == pygame.K_ESCAPE:
            self.running = False
            return

        if self.active_input is not None:
            self._handle_text_input_key(event)
            return

        if event.key == pygame.K_SPACE:
            self.playing = not self.playing
            self.batch_queue.clear()
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
        value = int(round(self.SPEED_MIN_MS + (self.SPEED_MAX_MS - self.SPEED_MIN_MS) * t))
        self.animation_duration_ms = value - (value % self.SPEED_STEP_MS)

    def _can_interact(self) -> bool:
        return not self.busy

    def _handle_panel_click(self, pos):
        if self.rects["algo_dropdown"].collidepoint(pos):
            self.algo_dropdown_open = not self.algo_dropdown_open
            return

        if self.algo_dropdown_open:
            for idx, algo in enumerate(self.algo_options):
                if self._algo_option_rect(idx).collidepoint(pos):
                    self.selected_algo = algo
                    self.algo_dropdown_open = False
                    self.status_text = f"Algorithm: {algo}"
                    return
            self.algo_dropdown_open = False

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
                self.batch_queue.clear()
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
                self.batch_queue = deque((m, i) for i, m in enumerate(self.scramble_moves))
                self.status_text = "Running scramble"
            return

        if self.rects["solve_only"].collidepoint(pos):
            self.pause_all()
            if self.solve_moves:
                start = len(self.scramble_moves)
                self.seq_index = max(self.seq_index, start)
                self.batch_queue = deque((m, start + i) for i, m in enumerate(self.solve_moves))
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
        self.batch_queue.clear()

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

        moves: List[str] = []
        prev_face = ""

        for _ in range(length):
            candidates = [face for face in self.SCRAMBLE_FACES if face != prev_face]
            face = random.choice(candidates)
            move = face + random.choice(self.SCRAMBLE_SUFFIXES)
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
            self._build_solver_command(solver_path, move_text),
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

        params = get_move_params(move)
        turns = 2 if move.endswith("2") else 1
        duration = max(80, int(self.animation_duration_ms)) * turns
        target_angle = (math.pi / 2) * int(params["direction"]) * int(params["turns"])

        self.current_move = move
        self.anim = {
            "move": move,
            "axis": str(params["axis"]),
            "layer": int(params["layer"]),
            "target": target_angle,
            "elapsed": 0.0,
            "duration": duration / 1000.0,
            "on_complete": on_complete,
        }

    def _build_solver_command(self, solver_path: Path, move_text: str) -> List[str]:
        return [str(solver_path), "-a", self.selected_algo, move_text]

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
            move, idx = self.batch_queue.popleft()

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
        ui.draw_button(self, surface, rect, text, enabled=enabled, accent=accent)

    def _draw_input(self, surface, label, key, rect, text):
        ui.draw_input(self, surface, label, key, rect, text)

    def _draw_speed_slider(self, surface):
        ui.draw_speed_slider(self, surface)

    def _draw_manual_grid(self, surface):
        ui.draw_manual_grid(self, surface)

    def _algo_option_rect(self, idx: int) -> pygame.Rect:
        return ui.algo_option_rect(self, idx)

    def _draw_algo_dropdown(self, surface):
        ui.draw_algo_dropdown(self, surface)

    def _draw_status(self, surface):
        ui.draw_status(self, surface)

    def _draw_move_barrel(self):
        ui.draw_move_barrel(self)

    def _draw_move_pill(self):
        ui.draw_move_pill(self)

    def render(self):
        ui.render(self)

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
