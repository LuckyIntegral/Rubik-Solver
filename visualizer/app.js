import { createScene } from './scene.js';
import { invertMove, parseMoves, VALID_MOVES } from './moves.js';

const $ = (id) => /** @type {HTMLElement} */(document.getElementById(id));

export function init() {
  const el = {
    canvas: /** @type {HTMLCanvasElement} */ ($('view')),
    scramble: /** @type {HTMLTextAreaElement} */ ($('scramble')),
    solve: /** @type {HTMLTextAreaElement} */ ($('solve')),
    err: $('error'),
    status: $('status'),
    pos: $('pos'),
    pillMoveB: $('pillMove').querySelector('b'),
    barrelInner: $('moveBarrelInner'),
    speed: /** @type {HTMLInputElement} */ ($('speed')),
    speedLabel: $('speedLabel'),
    manualGrid: $('manualGrid'),
    btn: {
      load: /** @type {HTMLButtonElement} */ ($('btnLoad')),
      resetCube: /** @type {HTMLButtonElement} */ ($('btnResetCube')),
      resetView: /** @type {HTMLButtonElement} */ ($('btnResetView')),
      back: /** @type {HTMLButtonElement} */ ($('btnBack')),
      step: /** @type {HTMLButtonElement} */ ($('btnStep')),
      play: /** @type {HTMLButtonElement} */ ($('btnPlay')),
      pause: /** @type {HTMLButtonElement} */ ($('btnPause')),
      scrambleOnly: /** @type {HTMLButtonElement} */ ($('btnScrambleOnly')),
      solveOnly: /** @type {HTMLButtonElement} */ ($('btnSolveOnly')),
    },
  };

  const scene = createScene({ canvas: el.canvas });

  const state = {
    sequence: /** @type {string[]} */ ([]),
    scrambleMoves: /** @type {string[]} */ ([]),
    solveMoves: /** @type {string[]} */ ([]),
    seqIndex: 0,
    applied: /** @type {string[]} */ ([]),
    playing: false,
    busy: false,
    timer: /** @type {ReturnType<typeof setTimeout> | null} */ (null),
  };

  const setError = (m) => { el.err.textContent = m || ''; };
  const setStatus = (m) => { el.status.textContent = m || ''; };
  const setPill = (m) => { if (el.pillMoveB) el.pillMoveB.textContent = m || '—'; };

  function updateHUD() {
    el.pos.textContent = `${state.seqIndex} / ${state.sequence.length}`;
    updateMoveBarrel();
  }

  function updateMoveBarrel() {
    if (!el.barrelInner) return;
    const center = state.seqIndex; // "next" move sits in the middle
    const depth = 5;
    const pieces = [];
    for (let i = -depth; i <= depth; i++) {
      const idx = center + i;
      const text = idx >= 0 && idx < state.sequence.length ? state.sequence[idx] : '—';
      pieces.push({ i, idx, text });
    }

    // Build once per update (small: 11 nodes). Super simple + deterministic.
    el.barrelInner.innerHTML = '';
    const frag = document.createDocumentFragment();

    for (const p of pieces) {
      const d = Math.abs(p.i);
      const opacity = Math.max(0, 1 - d * 0.16);
      const rot = p.i * 12;  // degrees
      const y = p.i * 30;    // px
      const z = 62 - d * 16; // px (fake curvature)

      const item = document.createElement('div');
      item.textContent = p.text;
      item.className = 'absolute left-1/2 top-1/2 -translate-x-1/2 -translate-y-1/2 font-mono text-bg tracking-wide';
      item.style.transform = `translateX(${y}px) translateZ(${z}px) rotateY(${rot}deg)`;
      item.style.opacity = String(opacity);
      if (p.idx === state.seqIndex) item.classList.add('border-b');
      frag.appendChild(item);
    }

    el.barrelInner.appendChild(frag);
  }

  function updateButtons() {
    const busy = state.busy;
    el.btn.load.disabled = busy;
    el.btn.resetCube.disabled = busy;
    el.btn.resetView.disabled = busy;
    el.btn.step.disabled = busy || state.seqIndex >= state.sequence.length;
    el.btn.back.disabled = busy || state.applied.length === 0;
    el.btn.play.disabled = busy || state.sequence.length === 0;
    el.btn.pause.disabled = busy || !state.playing;
    el.btn.scrambleOnly.disabled = busy || state.scrambleMoves.length === 0;
    el.btn.solveOnly.disabled = busy || state.solveMoves.length === 0;
    for (const b of el.manualGrid.querySelectorAll('button')) b.disabled = busy;
  }

  function setSpeedLabel() {
    el.speedLabel.textContent = `${Number(el.speed.value)} ms`;
  }
  el.speed.addEventListener('input', setSpeedLabel);
  setSpeedLabel();

  // Serialize all cube mutations so animations never overlap.
  let queue = Promise.resolve();
  function withBusy(fn) {
    queue = queue.then(async () => {
      state.busy = true;
      updateButtons();
      try { await fn(); }
      finally {
        state.busy = false;
        setPill('');
        updateHUD();
        updateButtons();
      }
    });
    return queue;
  }

  function pause() {
    state.playing = false;
    if (state.timer) { clearTimeout(state.timer); state.timer = null; }
    updateButtons();
  }

  async function applyOne(move) {
    await withBusy(async () => {
      setError('');
      setStatus(move);
      setPill(move);
      await scene.applyMoveAnimated(move, el.speed.value);
      state.applied.push(move);
    });
  }

  async function stepForward() {
    if (state.busy) return;
    if (state.seqIndex >= state.sequence.length) return;
    const m = state.sequence[state.seqIndex];
    await applyOne(m);
    state.seqIndex++;
    updateHUD();
    updateButtons();
  }

  async function undoOne() {
    if (state.busy) return;
    if (!state.applied.length) return;
    const last = state.applied.pop();
    await applyOne(invertMove(last));
    // invertMove is also recorded above; remove it so history stays "real moves"
    state.applied.pop();
    if (state.seqIndex > 0 && state.sequence[state.seqIndex - 1] === last) state.seqIndex--;
    updateHUD();
    updateButtons();
  }

  async function runMoves(moves, startIndex) {
    for (let i = 0; i < moves.length; i++) {
      if (!moves[i]) continue;
      // jump the cursor as we run
      state.seqIndex = Math.max(state.seqIndex, startIndex + i);
      await stepForward();
    }
  }

  async function playLoop() {
    if (!state.playing) return;
    if (state.busy) { state.timer = setTimeout(playLoop, 50); return; }
    if (state.seqIndex >= state.sequence.length) { pause(); setStatus('Done'); return; }
    try { await stepForward(); }
    catch (e) { pause(); setError(String(e?.message || e)); return; }
    state.timer = setTimeout(playLoop, 0);
  }

  function play() {
    if (!state.sequence.length) return;
    state.playing = true;
    setStatus('Playing');
    updateButtons();
    playLoop();
  }

  function loadSequence() {
    try {
      state.scrambleMoves = parseMoves(el.scramble.value);
      state.solveMoves = parseMoves(el.solve.value);
      state.sequence = [...state.scrambleMoves, ...state.solveMoves];
      state.seqIndex = 0;
      state.applied = [];
      setError('');
      setStatus(`Loaded: ${state.scrambleMoves.length} + ${state.solveMoves.length}`);
      updateHUD();
      updateButtons();
    } catch (e) {
      setError(String(e?.message || e));
    }
  }

  function resetCube() {
    pause();
    scene.resetCube();
    state.seqIndex = 0;
    state.applied = [];
    setError('');
    setStatus('Reset to solved');
    updateHUD();
    updateButtons();
  }

  function buildManualButtons() {
    const moves = ['R', "R'", 'R2', 'L', "L'", 'L2', 'U', "U'", 'U2', 'D', "D'", 'D2', 'F', "F'", 'F2', 'B', "B'", 'B2'];
    el.manualGrid.innerHTML = '';
    for (const m of moves) {
      const b = document.createElement('button');
      b.type = 'button';
      b.textContent = m;
      b.className = 'rounded-xl border border-white/10 bg-white/5 px-2 py-2 font-mono text-xs font-semibold hover:bg-white/10';
      b.addEventListener('click', () => { pause(); applyOne(m); });
      el.manualGrid.appendChild(b);
    }
  }

  el.btn.load.addEventListener('click', () => { pause(); loadSequence(); });
  el.btn.resetCube.addEventListener('click', resetCube);
  el.btn.resetView.addEventListener('click', () => { pause(); scene.resetView(); });
  el.btn.step.addEventListener('click', () => { pause(); stepForward(); });
  el.btn.back.addEventListener('click', () => { pause(); undoOne(); });
  el.btn.play.addEventListener('click', play);
  el.btn.pause.addEventListener('click', pause);
  el.btn.scrambleOnly.addEventListener('click', async () => {
    pause();
    resetCube();
    loadSequence();
    await runMoves(state.scrambleMoves, 0);
  });
  el.btn.solveOnly.addEventListener('click', async () => {
    pause();
    const solveStart = state.scrambleMoves.length;
    state.seqIndex = Math.max(state.seqIndex, solveStart);
    updateHUD();
    await runMoves(state.solveMoves, solveStart);
  });

  document.addEventListener('keydown', (e) => {
    const tag = (e.target && e.target.tagName) ? e.target.tagName.toUpperCase() : '';
    if (tag === 'INPUT' || tag === 'TEXTAREA' || tag === 'SELECT') return;
    if (e.key === ' ') { e.preventDefault(); state.playing ? pause() : play(); }
    if (e.key === 'ArrowRight') { e.preventDefault(); pause(); stepForward(); }
    if (e.key === 'ArrowLeft' || e.key === 'Backspace') { e.preventDefault(); pause(); undoOne(); }
    if (VALID_MOVES.has(e.key.toUpperCase())) { /* ignore */ }
  });

  // init
  scene.resetView();
  buildManualButtons();
  loadSequence();
  updateButtons();
}
