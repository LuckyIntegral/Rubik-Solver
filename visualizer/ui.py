"""UI layout and rendering helpers for the Rubik visualizer."""

import math

import pygame

UI_MARGIN = 12
UI_GAP = 8
BUTTON_HEIGHT = 34
INPUT_HEIGHT = 48
PANEL_TITLE_Y = 12
PANEL_SUBTITLE_Y = 36
STATUS_SOLVER_Y = 18
STATUS_MOVES_Y = 34
STATUS_LINE_Y = 56
STATUS_ERROR_Y = 78
MOVE_BARREL_DEPTH = 5
MOVE_BARREL_TOP_Y = 16
MOVE_BARREL_STEP_X = 30
MOVE_BARREL_STEP_Y = 4

COLOR_BG = (14, 16, 24)
COLOR_PANEL = (22, 25, 36)
COLOR_PANEL_BORDER = (88, 95, 132)
COLOR_CANVAS_BG = (24, 28, 40)


def setup_ui_rects(app):
    margin = UI_MARGIN
    gap = UI_GAP
    btn_h = BUTTON_HEIGHT

    app.panel_rect = pygame.Rect(0, 0, app.panel_width, app.height)
    app.canvas_rect = pygame.Rect(app.panel_width, 0, app.width - app.panel_width, app.height)

    y = margin + 52

    app.rects["scramble_input"] = pygame.Rect(margin, y, app.panel_width - margin * 2, INPUT_HEIGHT)
    y += INPUT_HEIGHT + gap + 26
    app.rects["solve_input"] = pygame.Rect(margin, y, app.panel_width - margin * 2, INPUT_HEIGHT)
    y += INPUT_HEIGHT + gap + 10
    app.rects["scramble_len_input"] = pygame.Rect(margin, y, 116, btn_h)
    app.rects["generate_scramble"] = pygame.Rect(margin + 116 + gap, y, app.panel_width - margin * 2 - 116 - gap, btn_h)
    y += btn_h + gap

    half_w = (app.panel_width - margin * 3) // 2
    app.rects["load"] = pygame.Rect(margin, y, half_w, btn_h)
    app.rects["reset_cube"] = pygame.Rect(margin + half_w + margin, y, half_w, btn_h)
    y += btn_h + gap

    app.rects["reset_view"] = pygame.Rect(margin, y, app.panel_width - margin * 2, btn_h)
    y += btn_h + 16

    app.rects["back"] = pygame.Rect(margin, y, half_w, btn_h)
    app.rects["step"] = pygame.Rect(margin + half_w + margin, y, half_w, btn_h)
    y += btn_h + gap

    app.rects["play"] = pygame.Rect(margin, y, half_w, btn_h)
    app.rects["pause"] = pygame.Rect(margin + half_w + margin, y, half_w, btn_h)
    y += btn_h + gap

    app.rects["demo"] = pygame.Rect(margin, y, app.panel_width - margin * 2, btn_h)
    y += btn_h + gap

    app.rects["scramble_only"] = pygame.Rect(margin, y, half_w, btn_h)
    app.rects["solve_only"] = pygame.Rect(margin + half_w + margin, y, half_w, btn_h)
    y += btn_h + gap

    app.rects["run_solver"] = pygame.Rect(margin, y, app.panel_width - margin * 2, btn_h)
    y += btn_h + gap

    # algorithm dropdown removed

    app.rects["speed_track"] = pygame.Rect(margin, y + 18, app.panel_width - margin * 2 - 90, 10)
    app.rects["speed_label"] = pygame.Rect(app.rects["speed_track"].right + 10, y + 7, 82, 22)
    y += 60

    app.manual_buttons = []
    cols = 6
    manual_w = (app.panel_width - margin * 2 - (cols - 1) * gap) // cols
    manual_h = 28
    for idx, move in enumerate(app.MANUAL_MOVES):
        row = idx // cols
        col = idx % cols
        rx = margin + col * (manual_w + gap)
        ry = y + row * (manual_h + gap)
        app.manual_buttons.append((pygame.Rect(rx, ry, manual_w, manual_h), move))
    y += (manual_h + gap) * 3 + 8

    app.rects["status"] = pygame.Rect(margin, y, app.panel_width - margin * 2, app.height - y - margin)



def draw_button(app, surface, rect, text, enabled=True, accent=False):
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
    label = app._fonts["body"].render(text, True, text_color)
    surface.blit(label, (rect.centerx - label.get_width() // 2, rect.centery - label.get_height() // 2))


def draw_input(app, surface, label, key, rect, text):
    label_surface = app._fonts["label"].render(label, True, (178, 184, 205))
    surface.blit(label_surface, (rect.left, rect.top - 18))

    active = app.active_input == key
    fill = (26, 28, 39) if not active else (34, 37, 52)
    border = (88, 95, 132) if not active else (122, 138, 210)
    pygame.draw.rect(surface, fill, rect, border_radius=10)
    pygame.draw.rect(surface, border, rect, 1, border_radius=10)

    content = text
    if active and pygame.time.get_ticks() % 900 < 500:
        content += "_"

    clip = surface.get_clip()
    surface.set_clip(rect.inflate(-10, -10))
    txt = app._fonts["mono"].render(content, True, (232, 235, 245))
    surface.blit(txt, (rect.left + 8, rect.centery - txt.get_height() // 2))
    surface.set_clip(clip)


def draw_speed_slider(app, surface):
    label = app._fonts["label"].render("Animation timeframe", True, (178, 184, 205))
    track = app.rects["speed_track"]
    surface.blit(label, (track.left, track.top - 20))

    pygame.draw.rect(surface, (42, 45, 58), track, border_radius=8)
    pygame.draw.rect(surface, (80, 86, 112), track, 1, border_radius=8)

    t = (app.animation_duration_ms - app.SPEED_MIN_MS) / float(app.SPEED_MAX_MS - app.SPEED_MIN_MS)
    t = max(0.0, min(1.0, t))
    knob_x = int(track.left + t * track.width)

    fill_track = pygame.Rect(track.left, track.top, max(1, knob_x - track.left), track.height)
    pygame.draw.rect(surface, (82, 146, 112), fill_track, border_radius=8)

    pygame.draw.circle(surface, (230, 236, 248), (knob_x, track.centery), 7)
    pygame.draw.circle(surface, (100, 108, 132), (knob_x, track.centery), 7, 1)

    speed_text = app._fonts["mono"].render(f"{app.animation_duration_ms} ms", True, (214, 220, 236))
    speed_rect = app.rects["speed_label"]
    surface.blit(speed_text, (speed_rect.right - speed_text.get_width(), speed_rect.top))


def draw_manual_grid(app, surface):
    if not app.manual_buttons:
        return

    first_rect = app.manual_buttons[0][0]
    label = app._fonts["label"].render("Manual move", True, (178, 184, 205))
    surface.blit(label, (first_rect.left, first_rect.top - 20))

    for rect, move in app.manual_buttons:
        draw_button(app, surface, rect, move, enabled=not app.busy)


# algorithm dropdown UI removed


def draw_status(app, surface):
    status_rect = app.rects["status"]

    pos = app._fonts["mono"].render(f"{app.seq_index} / {len(app.sequence)}", True, (216, 224, 243))
    surface.blit(pos, (status_rect.right - pos.get_width(), status_rect.top))

    if app.last_exec_time_ms is not None:
        elapsed = app._fonts["mono_small"].render(
            f"solver: {app.last_exec_time_ms:.2f} ms",
            True,
            (176, 212, 236),
        )
        surface.blit(elapsed, (status_rect.left, status_rect.top + STATUS_SOLVER_Y))

        moves = app._fonts["mono_small"].render(
            f"moves: {len(app.solve_moves)}",
            True,
            (176, 212, 236),
        )
        surface.blit(moves, (status_rect.left, status_rect.top + STATUS_MOVES_Y))

    status_line = app.status_text or "Ready"
    status_surf = app._fonts["mono"].render(status_line, True, (150, 220, 170))
    surface.blit(status_surf, (status_rect.left, status_rect.top + STATUS_LINE_Y))

    if app.error_text:
        err = app._fonts["mono"].render(app.error_text[:80], True, (238, 122, 122))
        surface.blit(err, (status_rect.left, status_rect.top + STATUS_ERROR_Y))


def draw_move_barrel(app):
    if app.canvas_rect.width <= 0 or app.canvas_rect.height <= 0:
        return

    center_x = app.canvas_rect.left + app.canvas_rect.width // 2
    top_y = app.canvas_rect.top + MOVE_BARREL_TOP_Y

    for offset in range(-MOVE_BARREL_DEPTH, MOVE_BARREL_DEPTH + 1):
        idx = app.seq_index + offset
        text = app.sequence[idx] if 0 <= idx < len(app.sequence) else "-"
        dist = abs(offset)
        alpha = max(35, 255 - dist * 40)
        color = (210, 220, 236, alpha)

        text_surf = app._fonts["mono"].render(text, True, (color[0], color[1], color[2]))
        x = center_x + offset * MOVE_BARREL_STEP_X - text_surf.get_width() // 2
        y = top_y + dist * MOVE_BARREL_STEP_Y
        app.screen.blit(text_surf, (x, y))

        if offset == 0:
            underline = pygame.Rect(x - 2, y + text_surf.get_height() + 2, text_surf.get_width() + 4, 2)
            pygame.draw.rect(app.screen, (194, 204, 230), underline)


def draw_move_pill(app):
    text = app.current_move if app.current_move else "-"
    body = app._fonts["mono"].render(f"Move: {text}", True, (228, 232, 244))
    pad_x, pad_y = 10, 7
    rect = pygame.Rect(
        app.canvas_rect.right - body.get_width() - pad_x * 2 - 16,
        app.canvas_rect.bottom - body.get_height() - pad_y * 2 - 16,
        body.get_width() + pad_x * 2,
        body.get_height() + pad_y * 2,
    )
    pygame.draw.rect(app.screen, (20, 24, 34), rect, border_radius=14)
    pygame.draw.rect(app.screen, (90, 98, 130), rect, 1, border_radius=14)
    app.screen.blit(body, (rect.left + pad_x, rect.top + pad_y))


def render(app):
    app.screen.fill(COLOR_BG)

    panel = pygame.Surface((app.panel_rect.width, app.panel_rect.height), pygame.SRCALPHA)
    panel.fill(COLOR_PANEL)

    title = app._fonts["title"].render("Rubik's Cube Visualizer", True, (230, 236, 248))
    subtitle = app._fonts["body"].render("Scramble + solve controls with 3D playback", True, (162, 168, 191))
    panel.blit(title, (UI_MARGIN, PANEL_TITLE_Y))
    panel.blit(subtitle, (UI_MARGIN, PANEL_SUBTITLE_Y))

    draw_input(app, panel, "Scramble algorithm", "scramble", app.rects["scramble_input"], app.scramble_text)
    draw_input(app, panel, "Solve algorithm", "solve", app.rects["solve_input"], app.solve_text)
    draw_input(app, panel, "", "scramble_len", app.rects["scramble_len_input"], app.scramble_len_text)
    draw_button(
        app,
        panel,
        app.rects["generate_scramble"],
        "Generate scramble",
        enabled=not app.busy,
    )

    draw_button(app, panel, app.rects["load"], "Load sequence", enabled=not app.busy)
    draw_button(app, panel, app.rects["reset_cube"], "Reset cube", enabled=not app.busy)
    draw_button(app, panel, app.rects["reset_view"], "Reset view", enabled=not app.busy)

    draw_button(app, panel, app.rects["back"], "Back", enabled=(not app.busy and bool(app.applied_moves)))
    draw_button(app, panel, app.rects["step"], "Step", enabled=(not app.busy and app.seq_index < len(app.sequence)))
    draw_button(app, panel, app.rects["play"], "Play", enabled=(not app.busy and bool(app.sequence)), accent=True)
    draw_button(app, panel, app.rects["pause"], "Pause", enabled=(not app.busy and app.playing))
    draw_button(
        app,
        panel,
        app.rects["demo"],
        "Demo",
        enabled=True,
        accent=app.demo_mode,
    )
    draw_button(
        app,
        panel,
        app.rects["scramble_only"],
        "Scramble only",
        enabled=(not app.busy and bool(app.scramble_moves)),
    )
    draw_button(
        app,
        panel,
        app.rects["solve_only"],
        "Solve only",
        enabled=(not app.busy and bool(app.solve_moves)),
    )
    draw_button(
        app,
        panel,
        app.rects["run_solver"],
        "Run ./rubik",
        enabled=(not app.busy and bool(app.scramble_text.strip())),
        accent=True,
    )

    draw_speed_slider(app, panel)
    draw_manual_grid(app, panel)
    draw_status(app, panel)

    app.screen.blit(panel, app.panel_rect.topleft)
    pygame.draw.rect(app.screen, COLOR_PANEL_BORDER, app.panel_rect, 1)

    canvas_surface = pygame.Surface((app.canvas_rect.width, app.canvas_rect.height))
    canvas_surface.fill(COLOR_CANVAS_BG)

    animation_payload = None
    if app.anim is not None:
        t = min(1.0, float(app.anim["elapsed"]) / max(1e-6, float(app.anim["duration"])))
        eased = 4 * t * t * t if t < 0.5 else 1 - ((-2 * t + 2) ** 3) / 2
        animation_payload = {
            "axis": str(app.anim["axis"]),
            "layer": int(app.anim["layer"]),
            "angle": float(app.anim["target"]) * eased,
        }

    app.renderer.render(canvas_surface, app.cube, animation_payload)
    app.screen.blit(canvas_surface, app.canvas_rect.topleft)

    pygame.draw.rect(app.screen, COLOR_PANEL_BORDER, app.canvas_rect, 1)
    draw_move_barrel(app)
    draw_move_pill(app)

    pygame.display.flip()
