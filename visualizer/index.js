'use strict';

const VALID_MOVES = new Set([
    'R', 'L', 'U', 'D', 'F', 'B',
    "R'", "L'", "U'", "D'", "F'", "B'",
    'R2', 'L2', 'U2', 'D2', 'F2', 'B2',
]);

const FACE_COLORS = {
    U: '#ffffff', D: '#ffd500',
    F: '#009b48', B: '#0046ad',
    L: '#ff5900', R: '#b71234',
};

function parseMoves(raw) {
    const tokens = raw.trim().split(/\s+/).filter(Boolean);
    const invalid = tokens.filter(t => !VALID_MOVES.has(t));
    if (invalid.length) throw new Error(`Invalid move(s): ${invalid.join(', ')}`);
    return tokens;
}

function initCube() {
    const s = [];
    for (const x of [-1, 0, 1])
        for (const z of [-1, 0, 1]) {
            s.push({ x, y: 1, z, nx: 0, ny: 1, nz: 0, color: FACE_COLORS.U });
            s.push({ x, y: -1, z, nx: 0, ny: -1, nz: 0, color: FACE_COLORS.D });
        }
    for (const x of [-1, 0, 1])
        for (const y of [-1, 0, 1]) {
            s.push({ x, y, z: 1, nx: 0, ny: 0, nz: 1, color: FACE_COLORS.F });
            s.push({ x, y, z: -1, nx: 0, ny: 0, nz: -1, color: FACE_COLORS.B });
        }
    for (const y of [-1, 0, 1])
        for (const z of [-1, 0, 1]) {
            s.push({ x: 1, y, z, nx: 1, ny: 0, nz: 0, color: FACE_COLORS.R });
            s.push({ x: -1, y, z, nx: -1, ny: 0, nz: 0, color: FACE_COLORS.L });
        }
    return s;
}

function cloneStickers(stickers) {
    return stickers.map(s => ({ ...s }));
}

const ROT = {
    x: (y, z, d) => d === 1 ? [-z, y] : [z, -y],
    y: (x, z, d) => d === 1 ? [z, -x] : [-z, x],
    z: (x, y, d) => d === 1 ? [-y, x] : [y, -x],
};

const MOVE_DEF = {
    R: { axis: 'x', layer: 1,  baseDir: 1  },
    L: { axis: 'x', layer: -1, baseDir: -1 },
    U: { axis: 'y', layer: 1,  baseDir: 1  },
    D: { axis: 'y', layer: -1, baseDir: -1 },
    F: { axis: 'z', layer: 1,  baseDir: -1 },
    B: { axis: 'z', layer: -1, baseDir: 1  },
};

function applyMove(stickers, move) {
    const base = move[0];
    const turns = move.endsWith('2') ? 2 : 1;
    const { axis, layer, baseDir } = MOVE_DEF[base];
    const dir = move.endsWith("'") ? -baseDir : baseDir;

    for (let t = 0; t < turns; t++) {
        for (const s of stickers) {
            if (s[axis] !== layer) continue;
            if (axis === 'x') {
                [s.y, s.z]   = ROT.x(s.y,  s.z,  dir);
                [s.ny, s.nz] = ROT.x(s.ny, s.nz, dir);
            } else if (axis === 'y') {
                [s.x, s.z]   = ROT.y(s.x,  s.z,  dir);
                [s.nx, s.nz] = ROT.y(s.nx, s.nz, dir);
            } else {
                [s.x, s.y]   = ROT.z(s.x,  s.y,  dir);
                [s.nx, s.ny] = ROT.z(s.nx, s.ny, dir);
            }
        }
    }
}

function buildSnapshots(moves) {
    const cube = initCube();
    const snaps = [cloneStickers(cube)];
    for (const m of moves) {
        applyMove(cube, m);
        snaps.push(cloneStickers(cube));
    }
    return snaps;
}

const H = 0.47;

function stickerCorners(s) {
    const { x, y, z, ny, nz } = s;
    if (ny !== 0)
        return [[x - H, y, z - H], [x + H, y, z - H], [x + H, y, z + H], [x - H, y, z + H]];
    if (nz !== 0)
        return [[x - H, y - H, z], [x + H, y - H, z], [x + H, y + H, z], [x - H, y + H, z]];
    return [[x, y - H, z - H], [x, y - H, z + H], [x, y + H, z + H], [x, y + H, z - H]];
}

function makeRotMat(azDeg, elDeg) {
    const az = azDeg * Math.PI / 180;
    const el = elDeg * Math.PI / 180;
    const ca = Math.cos(az), sa = Math.sin(az);
    const ce = Math.cos(el), se = Math.sin(el);
    return [
        [ca,       0,   sa      ],
        [sa * se,  ce, -se * ca ],
        [-sa * ce, se,  ce * ca ],
    ];
}

function transformPt(m, [x, y, z]) {
    return [
        m[0][0] * x + m[0][1] * y + m[0][2] * z,
        m[1][0] * x + m[1][1] * y + m[1][2] * z,
        m[2][0] * x + m[2][1] * y + m[2][2] * z,
    ];
}

const BG = 1.40;
const FACE_FRAMES = [
    [[-BG, 1, -BG], [BG, 1, -BG], [BG, 1,  BG], [-BG, 1,  BG]],
    [[-BG,-1, -BG], [BG,-1, -BG], [BG,-1,  BG], [-BG,-1,  BG]],
    [[-BG,-BG, 1],  [BG,-BG, 1],  [BG, BG, 1],  [-BG, BG, 1] ],
    [[-BG,-BG,-1],  [BG,-BG,-1],  [BG, BG,-1],  [-BG, BG,-1] ],
    [[1, -BG,-BG],  [1, -BG, BG], [1,  BG, BG], [1,   BG,-BG]],
    [[-1,-BG,-BG],  [-1,-BG, BG], [-1, BG, BG], [-1,  BG,-BG]],
];

function drawScene(ctx, stickers, azimuth, elevation) {
    const W = ctx.canvas.width;
    const Ht = ctx.canvas.height;
    const scale = Math.min(W, Ht) / 6.5;
    const cx = W / 2, cy = Ht / 2;
    const m = makeRotMat(azimuth, elevation);

    const quads = [];

    for (const frame of FACE_FRAMES) {
        const pts3 = frame.map(p => transformPt(m, p));
        const avgZ = pts3.reduce((s, p) => s + p[2], 0) / 4;
        quads.push({ pts: pts3.map(([px, py]) => [cx + px * scale, cy - py * scale]), avgZ, color: '#1a1a1a', stroke: false });
    }

    for (const s of stickers) {
        const corners = stickerCorners(s).map(p => transformPt(m, p));
        const avgZ = corners.reduce((sum, p) => sum + p[2], 0) / 4;
        quads.push({ pts: corners.map(([px, py]) => [cx + px * scale, cy - py * scale]), avgZ, color: s.color, stroke: true });
    }

    quads.sort((a, b) => a.avgZ - b.avgZ);

    ctx.clearRect(0, 0, W, Ht);
    for (const q of quads) {
        ctx.beginPath();
        ctx.moveTo(q.pts[0][0], q.pts[0][1]);
        for (let i = 1; i < q.pts.length; i++) ctx.lineTo(q.pts[i][0], q.pts[i][1]);
        ctx.closePath();
        ctx.fillStyle = q.color;
        ctx.fill();
        if (q.stroke) {
            ctx.strokeStyle = '#111';
            ctx.lineWidth = 1.2;
            ctx.stroke();
        }
    }
}

const state = {
    snapshots: [initCube()],
    moves: [],
    shuffleCount: 0,
    index: 0,
    timer: null,
    azimuth: 30,
    elevation: 25,
};

const canvas      = document.getElementById('cube-canvas');
const ctx         = canvas.getContext('2d');
const inputShuffle = document.getElementById('input-shuffle');
const inputSolve   = document.getElementById('input-solve');
const errorMsg    = document.getElementById('error-msg');
const btnApply    = document.getElementById('btn-apply');
const btnResetView = document.getElementById('btn-reset-view');
const btnPlay     = document.getElementById('btn-play');
const btnPause    = document.getElementById('btn-pause');
const btnBack     = document.getElementById('btn-back');
const btnForward  = document.getElementById('btn-forward');
const speedSlider = document.getElementById('speed-slider');
const speedLabel  = document.getElementById('speed-label');
const stepDisplay = document.getElementById('step-display');
const moveDisplay = document.getElementById('move-display');
const progressBar = document.getElementById('progress-bar');
const seqDisplay  = document.getElementById('seq-display');

function render() {
    drawScene(ctx, state.snapshots[state.index], state.azimuth, state.elevation);
}

const PILL_BASE = 'inline-block px-1.5 py-px rounded mx-px border text-xs cursor-default transition-opacity';
const PILL_SHUFFLE = 'bg-[#2a2a2a] border-[#555] text-[#ccc]';
const PILL_SOLVE   = 'bg-[#2a2a2a] border-[#4070c8] text-[#88aaee]';
const PILL_ACTIVE  = '!bg-[#4070c8] !border-[#4070c8] !text-white';
const PILL_DONE    = 'opacity-40';

function updateUI() {
    const total = state.moves.length;
    const i = state.index;

    stepDisplay.textContent = total === 0
        ? 'Step — / —'
        : `Step ${i} / ${total}`;

    if (total === 0) {
        moveDisplay.textContent = 'No sequence loaded';
    } else if (i === 0) {
        moveDisplay.textContent = 'Start position';
    } else {
        const move = state.moves[i - 1];
        const phase = (i <= state.shuffleCount) ? '🔀 Shuffle' : '✅ Solve';
        moveDisplay.textContent = `${phase}  →  ${move}`;
    }

    const pct = total === 0 ? 0 : (i / total) * 100;
    progressBar.style.width = `${pct}%`;

    if (total > 0) {
        seqDisplay.innerHTML = state.moves.map((m, idx) => {
            const base    = idx < state.shuffleCount ? PILL_SHUFFLE : PILL_SOLVE;
            const active  = idx === i - 1 ? PILL_ACTIVE : '';
            const done    = idx < i && idx !== i - 1 ? PILL_DONE : '';
            return `<span class="${PILL_BASE} ${base} ${active} ${done}">${m}</span>`;
        }).join(' ');
    } else {
        seqDisplay.innerHTML = '—';
    }
}

function pause() {
    if (state.timer !== null) { clearTimeout(state.timer); state.timer = null; }
}

function stepForward() {
    pause();
    if (state.index >= state.snapshots.length - 1) return;
    state.index++;
    render();
    updateUI();
}

function stepBack() {
    pause();
    if (state.index <= 0) return;
    state.index--;
    render();
    updateUI();
}

function tick() {
    state.timer = null;
    if (state.index >= state.snapshots.length - 1) return;
    state.index++;
    render();
    updateUI();
    const delay = Math.max(100, Number(speedSlider.value));
    state.timer = setTimeout(tick, delay);
}

function play() {
    if (state.timer !== null) return;
    if (state.snapshots.length <= 1) return;
    if (state.index >= state.snapshots.length - 1) state.index = 0;
    state.timer = setTimeout(tick, 0);
}

function applySequence() {
    errorMsg.textContent = '';
    let shuffleMoves, solveMoves;
    try {
        shuffleMoves = parseMoves(inputShuffle.value);
        solveMoves   = parseMoves(inputSolve.value);
    } catch (e) {
        errorMsg.textContent = e.message;
        return;
    }

    pause();
    state.moves        = [...shuffleMoves, ...solveMoves];
    state.shuffleCount = shuffleMoves.length;
    state.snapshots    = buildSnapshots(state.moves);
    state.index        = 0;
    render();
    updateUI();
}

let drag = null;

canvas.addEventListener('pointerdown', e => {
    canvas.setPointerCapture(e.pointerId);
    drag = { x: e.clientX, y: e.clientY, az: state.azimuth, el: state.elevation };
    e.preventDefault();
});

canvas.addEventListener('pointermove', e => {
    if (!drag) return;
    const dx = e.clientX - drag.x;
    const dy = e.clientY - drag.y;
    state.azimuth   = drag.az + dx * 0.4;
    state.elevation = Math.max(-89, Math.min(89, drag.el - dy * 0.4));
    render();
    e.preventDefault();
});

window.addEventListener('pointerup', () => { drag = null; });

btnApply.addEventListener('click', applySequence);
btnPlay.addEventListener('click', play);
btnPause.addEventListener('click', pause);
btnBack.addEventListener('click', stepBack);
btnForward.addEventListener('click', stepForward);

btnResetView.addEventListener('click', () => {
    state.azimuth = 30; state.elevation = 25; render();
});

speedSlider.addEventListener('input', () => {
    speedLabel.textContent = `${speedSlider.value} ms`;
});

document.addEventListener('keydown', e => {
    if (['INPUT', 'TEXTAREA'].includes(e.target.tagName)) return;
    if (e.key === ' ')          { e.preventDefault(); state.timer ? pause() : play(); }
    if (e.key === 'ArrowRight') { e.preventDefault(); stepForward(); }
    if (e.key === 'ArrowLeft')  { e.preventDefault(); stepBack(); }
});

[inputShuffle, inputSolve].forEach(el =>
    el.addEventListener('keydown', e => { if (e.key === 'Enter') applySequence(); })
);

applySequence();
