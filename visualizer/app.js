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
    prev: $('prevMove'),
    cur: $('curMove'),
    next: $('nextMove'),
    seqStrip: $('seqStrip'),
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
    const prev = state.seqIndex >= 2 ? state.sequence[state.seqIndex - 2] : (state.seqIndex === 1 ? state.sequence[0] : null);
    const cur = state.applied.length ? state.applied[state.applied.length - 1] : null;
    const next = state.seqIndex < state.sequence.length ? state.sequence[state.seqIndex] : null;
    el.prev.textContent = prev || '—';
    el.cur.textContent = cur || '—';
    el.next.textContent = next || '—';
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

  function renderSeqStrip() {
    el.seqStrip.innerHTML = '';
    const frag = document.createDocumentFragment();
    state.sequence.forEach((m, idx) => {
      const chip = document.createElement('span');
      chip.dataset.idx = String(idx);
      chip.textContent = m;
      chip.className = 'shrink-0 select-none whitespace-nowrap rounded-full border border-white/10 bg-white/5 px-2 py-1.5 font-mono text-xs text-gray-100';
      frag.appendChild(chip);
    });
    el.seqStrip.appendChild(frag);
  }

  function highlightSeqStrip() {
    for (const chip of el.seqStrip.children) {
      const idx = Number(chip.dataset.idx);
      chip.style.opacity = idx < state.seqIndex ? '0.45' : '1';
      chip.style.outline = idx === state.seqIndex ? '2px solid rgba(125,211,252,.65)' : 'none';
      chip.style.outlineOffset = '2px';
    }
  }

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
        highlightSeqStrip();
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
    highlightSeqStrip();
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
    highlightSeqStrip();
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
      renderSeqStrip();
      updateHUD();
      highlightSeqStrip();
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
    highlightSeqStrip();
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
    highlightSeqStrip();
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
