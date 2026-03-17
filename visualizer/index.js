import * as THREE from 'https://esm.sh/three@0.161.0';
import { OrbitControls } from 'https://esm.sh/three@0.161.0/examples/jsm/controls/OrbitControls.js';

const VALID_MOVES = new Set([
  'R', 'L', 'U', 'D', 'F', 'B',
  "R'", "L'", "U'", "D'", "F'", "B'",
  'R2', 'L2', 'U2', 'D2', 'F2', 'B2',
]);

function parseMoves(raw) {
  const tokens = raw.trim().split(/\s+/).filter(Boolean);
  const invalid = tokens.filter(t => !VALID_MOVES.has(t));
  if (invalid.length) throw new Error(`Invalid move(s): ${invalid.join(', ')}`);
  return tokens;
}

function invertMove(m) {
  if (m.endsWith('2')) return m;
  return m.endsWith("'") ? m.slice(0, -1) : `${m}'`;
}

const MOVE_DEF = {
  R: { axis: 'x', layer: 1, baseDir: 1 },
  L: { axis: 'x', layer: -1, baseDir: -1 },
  U: { axis: 'y', layer: 1, baseDir: 1 },
  D: { axis: 'y', layer: -1, baseDir: -1 },
  F: { axis: 'z', layer: 1, baseDir: -1 },
  B: { axis: 'z', layer: -1, baseDir: 1 },
};

const FACE = {
  U: 0xFFFFFF,
  D: 0xFFD500,
  F: 0x009B48,
  B: 0x0046AD,
  L: 0xFF5900,
  R: 0xB71234,
};

const canvas = document.getElementById('view');
const elScramble = document.getElementById('scramble');
const elSolve = document.getElementById('solve');
const elErr = document.getElementById('error');
const elStatus = document.getElementById('status');
const elPos = document.getElementById('pos');
const elPillMove = document.getElementById('pillMove');
const elPrevMove = document.getElementById('prevMove');
const elCurMove = document.getElementById('curMove');
const elNextMove = document.getElementById('nextMove');
const elSeqStrip = document.getElementById('seqStrip');

const btnLoad = document.getElementById('btnLoad');
const btnResetCube = document.getElementById('btnResetCube');
const btnResetView = document.getElementById('btnResetView');
const btnBack = document.getElementById('btnBack');
const btnStep = document.getElementById('btnStep');
const btnPlay = document.getElementById('btnPlay');
const btnPause = document.getElementById('btnPause');
const btnScrambleOnly = document.getElementById('btnScrambleOnly');
const btnSolveOnly = document.getElementById('btnSolveOnly');
const elSpeed = document.getElementById('speed');
const elSpeedLabel = document.getElementById('speedLabel');
const elManualGrid = document.getElementById('manualGrid');
const elMini = document.getElementById('mini');
const elMiniSize = document.getElementById('miniSize');
const elMiniSizeLabel = document.getElementById('miniSizeLabel');
const elSplitter = document.getElementById('splitter');
const elLeftPanel = document.getElementById('leftPanel');

const state = {
  sequence: [],
  scrambleMoves: [],
  solveMoves: [],
  seqIndex: 0,
  playing: false,
  busy: false,
  playTimer: null,
  appliedHistory: [], // actual moves applied in the current cube state (including manual)
  executingSeqIndex: null, // index in sequence currently animating
};

function setError(msg) {
  elErr.textContent = msg || '';
}

function setStatus(msg) {
  elStatus.textContent = msg;
}

function setMovePill(m) {
  const b = elPillMove.querySelector('b');
  b.textContent = m || '—';
}

function setSpeedLabel() {
  elSpeedLabel.textContent = `${Number(elSpeed.value)} ms`;
}
setSpeedLabel();
elSpeed.addEventListener('input', setSpeedLabel);

function setMiniSizeLabel() {
  elMiniSizeLabel.textContent = `${Number(elMiniSize.value)} px`;
  const px = Math.max(80, Number(elMiniSize.value));
  elMini.style.width = `${px}px`;
  elMini.style.height = `${px}px`;
}
setMiniSizeLabel();
elMiniSize.addEventListener('input', setMiniSizeLabel);

// --- THREE scene
const renderer = new THREE.WebGLRenderer({ canvas, antialias: true, alpha: true });
renderer.setPixelRatio(Math.min(2, window.devicePixelRatio || 1));
renderer.outputColorSpace = THREE.SRGBColorSpace;

const scene = new THREE.Scene();
scene.fog = new THREE.Fog(0x07080b, 8, 24);

const camera = new THREE.PerspectiveCamera(38, 1, 0.1, 100);
camera.position.set(7.5, 6.0, 7.5);

const miniCanvas = document.getElementById('miniView');
const miniRenderer = new THREE.WebGLRenderer({ canvas: miniCanvas, antialias: true, alpha: true });
miniRenderer.setPixelRatio(Math.min(2, window.devicePixelRatio || 1));
miniRenderer.outputColorSpace = THREE.SRGBColorSpace;
const miniCamera = new THREE.PerspectiveCamera(38, 1, 0.1, 100);

const controls = new OrbitControls(camera, canvas);
controls.enableDamping = true;
controls.dampingFactor = 0.075;
controls.enablePan = false;
controls.minDistance = 6.0;
controls.maxDistance = 18.0;

const amb = new THREE.AmbientLight(0xffffff, 0.55);
scene.add(amb);
const key = new THREE.DirectionalLight(0xffffff, 1.05);
key.position.set(8, 10, 6);
scene.add(key);
const fill = new THREE.DirectionalLight(0x9bbcff, 0.45);
fill.position.set(-10, 5, -6);
scene.add(fill);

const cubeRoot = new THREE.Group();
scene.add(cubeRoot);

function makeStickerMaterial(color) {
  return new THREE.MeshStandardMaterial({
    color,
    roughness: 0.55,
    metalness: 0.05,
    emissive: 0x000000,
  });
}

const matCore = new THREE.MeshStandardMaterial({ color: 0x0a0a0a, roughness: 0.95, metalness: 0.0 });
const matBlank = new THREE.MeshStandardMaterial({ color: 0x101010, roughness: 0.95, metalness: 0.0 });

const matU = makeStickerMaterial(FACE.U);
const matD = makeStickerMaterial(FACE.D);
const matF = makeStickerMaterial(FACE.F);
const matB = makeStickerMaterial(FACE.B);
const matL = makeStickerMaterial(FACE.L);
const matR = makeStickerMaterial(FACE.R);

function materialsForCubie(x, y, z) {
  // BoxGeometry face order: +X, -X, +Y, -Y, +Z, -Z
  return [
    x === 1 ? matR : matBlank,
    x === -1 ? matL : matBlank,
    y === 1 ? matU : matBlank,
    y === -1 ? matD : matBlank,
    z === 1 ? matF : matBlank,
    z === -1 ? matB : matBlank,
  ];
}

const cubies = [];
const spacing = 1.05;
const geom = new THREE.BoxGeometry(0.98, 0.98, 0.98);
for (const x of [-1, 0, 1]) for (const y of [-1, 0, 1]) for (const z of [-1, 0, 1]) {
  const mesh = new THREE.Mesh(geom, materialsForCubie(x, y, z));
  mesh.position.set(x * spacing, y * spacing, z * spacing);
  mesh.userData.coord = { x, y, z };
  mesh.userData.homeCoord = { x, y, z };
  cubeRoot.add(mesh);
  cubies.push(mesh);
}

// subtle frame around cube
{
  const edges = new THREE.EdgesGeometry(new THREE.BoxGeometry(3.25, 3.25, 3.25));
  const line = new THREE.LineSegments(edges, new THREE.LineBasicMaterial({ color: 0x1f2937, transparent: true, opacity: 0.9 }));
  cubeRoot.add(line);
}

function resize() {
  const { clientWidth: w, clientHeight: h } = canvas;
  if (w === 0 || h === 0) return;
  renderer.setSize(w, h, false);
  camera.aspect = w / h;
  camera.updateProjectionMatrix();
}
new ResizeObserver(resize).observe(canvas);
resize();

function resizeMini() {
  const { clientWidth: w, clientHeight: h } = miniCanvas;
  if (w === 0 || h === 0) return;
  miniRenderer.setSize(w, h, false);
  miniCamera.aspect = w / h;
  miniCamera.updateProjectionMatrix();
}
new ResizeObserver(resizeMini).observe(miniCanvas);
resizeMini();

function animateFrame() {
  controls.update();
  renderer.render(scene, camera);
  // mirrored "back view" camera
  miniCamera.position.copy(camera.position).multiplyScalar(-1);
  miniCamera.up.copy(camera.up);
  miniCamera.lookAt(0, 0, 0);
  miniRenderer.render(scene, miniCamera);
  requestAnimationFrame(animateFrame);
}
requestAnimationFrame(animateFrame);

// --- move animation helpers
function cubiesInLayer(axis, layer) {
  return cubies.filter(c => c.userData.coord[axis] === layer);
}

function roundCoord(n) {
  if (n > 0.5) return 1;
  if (n < -0.5) return -1;
  return 0;
}

function normalizeCubieTransform(c) {
  cubeRoot.attach(c); // preserve world transform while reparenting

  // snap position to grid
  c.position.set(
    roundCoord(c.position.x / spacing) * spacing,
    roundCoord(c.position.y / spacing) * spacing,
    roundCoord(c.position.z / spacing) * spacing,
  );
  c.userData.coord = {
    x: roundCoord(c.position.x / spacing),
    y: roundCoord(c.position.y / spacing),
    z: roundCoord(c.position.z / spacing),
  };

  // snap rotation to nearest 90deg on each axis
  const e = new THREE.Euler().setFromQuaternion(c.quaternion, 'XYZ');
  const snap = a => Math.round(a / (Math.PI / 2)) * (Math.PI / 2);
  e.set(snap(e.x), snap(e.y), snap(e.z));
  c.quaternion.setFromEuler(e);
}

function axisVector(axis) {
  if (axis === 'x') return new THREE.Vector3(1, 0, 0);
  if (axis === 'y') return new THREE.Vector3(0, 1, 0);
  return new THREE.Vector3(0, 0, 1);
}

function withLockedUI(fn) {
  state.busy = true;
  updateButtons();
  return Promise.resolve()
    .then(fn)
    .finally(() => { state.busy = false; state.executingSeqIndex = null; updateButtons(); updateSequenceUI(); });
}

function applyMoveAnimated(move) {
  if (!VALID_MOVES.has(move)) return Promise.reject(new Error(`Invalid move: ${move}`));
  const base = move[0];
  const turns = move.endsWith('2') ? 2 : 1;
  const { axis, layer, baseDir } = MOVE_DEF[base];
  const dir = move.endsWith("'") ? -baseDir : baseDir;

  const doQuarterTurn = () => new Promise(resolve => {
    const pivot = new THREE.Group();
    cubeRoot.add(pivot);

    const layerCubies = cubiesInLayer(axis, layer);
    for (const c of layerCubies) pivot.attach(c);

    const dur = Math.max(60, Number(elSpeed.value));
    const start = performance.now();
    const target = (Math.PI / 2) * dir;
    const ax = axisVector(axis);

    const step = (now) => {
      const t = Math.min(1, (now - start) / dur);
      const eased = t < 0.5 ? 4 * t * t * t : 1 - Math.pow(-2 * t + 2, 3) / 2; // cubic in/out
      pivot.setRotationFromAxisAngle(ax, target * eased);
      if (t < 1) requestAnimationFrame(step);
      else {
        pivot.setRotationFromAxisAngle(ax, target);
        // detach + normalize
        for (const c of layerCubies) normalizeCubieTransform(c);
        cubeRoot.remove(pivot);
        resolve();
      }
    };
    requestAnimationFrame(step);
  });

  return withLockedUI(async () => {
    setMovePill(move);
    for (let i = 0; i < turns; i++) await doQuarterTurn();
    setMovePill('');
  });
}

function renderSequenceStrip() {
  elSeqStrip.innerHTML = '';
  if (!state.sequence.length) return;
  const frag = document.createDocumentFragment();
  state.sequence.forEach((m, idx) => {
    const span = document.createElement('span');
    span.className = 'shrink-0 select-none whitespace-nowrap rounded-full border border-white/10 bg-white/5 px-2 py-1.5 font-mono text-xs text-gray-100';
    span.dataset.idx = String(idx);
    span.textContent = m;
    frag.appendChild(span);
  });
  elSeqStrip.appendChild(frag);
}

function updateSequenceUI() {
  const histLast = state.appliedHistory.length ? state.appliedHistory[state.appliedHistory.length - 1] : null;
  const prev = state.seqIndex >= 2 ? state.sequence[state.seqIndex - 2] : (state.seqIndex === 1 ? state.sequence[0] : null);
  const next = state.seqIndex < state.sequence.length ? state.sequence[state.seqIndex] : null;

  elPrevMove.textContent = prev || '—';
  elCurMove.textContent = histLast || '—';
  elNextMove.textContent = next || '—';

  const base = 'shrink-0 select-none whitespace-nowrap rounded-full border px-2 py-1.5 font-mono text-xs';
  const baseDefault = `${base} border-white/10 bg-white/5 text-gray-100`;
  const baseDone = `${base} border-white/10 bg-white/5 text-gray-100 opacity-45`;
  const baseNext = `${base} border-sky-300/60 bg-sky-300/15 text-gray-100`;
  const baseExec = `${base} border-emerald-300/60 bg-emerald-300/15 text-gray-100`;

  for (const chip of elSeqStrip.children) {
    const idx = Number(chip.dataset.idx);
    const isExec = idx === state.executingSeqIndex;
    const isNext = idx === state.seqIndex;
    const isDone = idx < state.seqIndex;
    chip.className = isExec ? baseExec : (isNext ? baseNext : (isDone ? baseDone : baseDefault));
  }

  const activeIdx = state.executingSeqIndex ?? state.seqIndex;
  if (activeIdx != null && activeIdx >= 0 && activeIdx < state.sequence.length) {
    const el = elSeqStrip.querySelector(`[data-idx="${activeIdx}"]`);
    if (el) el.scrollIntoView({ block: 'nearest', inline: 'center' });
  }
}

function resetCube() {
  state.sequence = [];
  state.seqIndex = 0;
  state.playing = false;
  state.appliedHistory = [];
  state.executingSeqIndex = null;
  if (state.playTimer) { clearTimeout(state.playTimer); state.playTimer = null; }

  for (const c of cubies) {
    const { x, y, z } = c.userData.homeCoord;
    c.position.set(x * spacing, y * spacing, z * spacing);
    c.rotation.set(0, 0, 0);
    c.userData.coord = { x, y, z };
  }
  setError('');
  setStatus('Reset to solved');
  updateHUD();
  updateButtons();
  renderSequenceStrip();
  updateSequenceUI();
}

function resetView() {
  controls.reset();
  camera.position.set(7.5, 6.0, 7.5);
  controls.target.set(0, 0, 0);
  controls.update();
}

function updateHUD() {
  elPos.textContent = `${state.seqIndex} / ${state.sequence.length}`;
}

function updateButtons() {
  const canStepFwd = !state.busy && state.seqIndex < state.sequence.length;
  const canUndo = !state.busy && state.appliedHistory.length > 0;
  const canScrambleOnly = !state.busy && state.scrambleMoves.length > 0;
  const canSolveOnly = !state.busy && state.solveMoves.length > 0;

  btnLoad.disabled = state.busy;
  btnResetCube.disabled = state.busy;
  btnResetView.disabled = state.busy;
  // manual buttons
  for (const b of elManualGrid.querySelectorAll('button')) b.disabled = state.busy;
  btnStep.disabled = !canStepFwd;
  btnBack.disabled = !canUndo;
  btnPlay.disabled = state.busy || state.sequence.length === 0;
  btnPause.disabled = state.busy || !state.playing;
  btnScrambleOnly.disabled = !canScrambleOnly;
  btnSolveOnly.disabled = !canSolveOnly;
}

async function runMoves(moves, startSeqIndex) {
  if (state.busy) return;
  for (let i = 0; i < moves.length; i++) {
    const m = moves[i];
    state.executingSeqIndex = startSeqIndex + i;
    updateSequenceUI();
    setStatus(`Run: ${m}`);
    setError('');
    await applyMoveAnimated(m);
    state.appliedHistory.push(m);
    state.seqIndex = Math.max(state.seqIndex, startSeqIndex + i + 1);
    updateHUD();
    updateButtons();
    updateSequenceUI();
  }
}

async function stepForward() {
  if (state.busy) return;
  if (state.seqIndex >= state.sequence.length) return;
  const m = state.sequence[state.seqIndex];
  setStatus(`Step: ${m}`);
  setError('');
  state.executingSeqIndex = state.seqIndex;
  updateSequenceUI();
  await applyMoveAnimated(m);
  state.appliedHistory.push(m);
  state.seqIndex++;
  updateHUD();
  updateButtons();
  updateSequenceUI();
}

async function undoOne() {
  if (state.busy) return;
  if (state.appliedHistory.length === 0) return;
  const last = state.appliedHistory.pop();
  const inv = invertMove(last);
  setStatus(`Undo: ${last}`);
  setError('');
  await applyMoveAnimated(inv);
  if (state.seqIndex > 0 && state.sequence[state.seqIndex - 1] === last) state.seqIndex--;
  updateHUD();
  updateButtons();
  updateSequenceUI();
}

function pause() {
  state.playing = false;
  if (state.playTimer) { clearTimeout(state.playTimer); state.playTimer = null; }
  setStatus('Paused');
  updateButtons();
}

async function playLoop() {
  if (!state.playing) return;
  if (state.busy) { state.playTimer = setTimeout(playLoop, 50); return; }
  if (state.seqIndex >= state.sequence.length) { pause(); setStatus('Done'); return; }
  try {
    await stepForward();
  } catch (e) {
    pause();
    setError(String(e?.message || e));
    return;
  }
  state.playTimer = setTimeout(playLoop, 0);
}

function play() {
  if (state.busy) return;
  if (state.sequence.length === 0) return;
  state.playing = true;
  setStatus('Playing');
  updateButtons();
  playLoop();
}

function loadSequence() {
  setError('');
  let scramble, solve;
  try {
    scramble = parseMoves(elScramble.value);
    solve = parseMoves(elSolve.value);
  } catch (e) {
    setError(String(e?.message || e));
    return;
  }
  state.scrambleMoves = scramble;
  state.solveMoves = solve;
  state.sequence = [...scramble, ...solve];
  state.seqIndex = 0;
  state.executingSeqIndex = null;
  setStatus(`Loaded: ${scramble.length} scramble + ${solve.length} solve`);
  updateHUD();
  updateButtons();
  renderSequenceStrip();
  updateSequenceUI();
}

async function applyManualMove(m) {
  if (state.busy) return;
  if (!VALID_MOVES.has(m)) { setError(`Invalid move: ${m}`); return; }
  setStatus(`Manual: ${m}`);
  setError('');
  await applyMoveAnimated(m);
  state.appliedHistory.push(m);
  updateHUD();
  updateButtons();
  updateSequenceUI();
}

btnLoad.addEventListener('click', loadSequence);
btnResetCube.addEventListener('click', resetCube);
btnResetView.addEventListener('click', resetView);
btnStep.addEventListener('click', () => { pause(); stepForward(); });
btnBack.addEventListener('click', () => { pause(); undoOne(); });
btnPlay.addEventListener('click', play);
btnPause.addEventListener('click', pause);
btnScrambleOnly.addEventListener('click', async () => {
  pause();
  if (state.busy) return;
  resetCube();
  // keep loaded sequence; resetCube clears it, so restore UI state
  // (scramble/solve inputs remain, so just reload parsing)
  loadSequence();
  await runMoves(state.scrambleMoves, 0);
});
btnSolveOnly.addEventListener('click', async () => {
  pause();
  if (state.busy) return;
  // If the user is earlier in the sequence, jump the cursor to the solve section,
  // but do not mutate the cube state (solver should run "from current state").
  const solveStart = state.scrambleMoves.length;
  state.seqIndex = Math.max(state.seqIndex, solveStart);
  updateHUD();
  updateSequenceUI();
  await runMoves(state.solveMoves, solveStart);
});

document.addEventListener('keydown', (e) => {
  const tag = (e.target && e.target.tagName) ? e.target.tagName.toUpperCase() : '';
  if (tag === 'INPUT' || tag === 'TEXTAREA' || tag === 'SELECT') return;
  if (e.key === ' ') { e.preventDefault(); state.playing ? pause() : play(); }
  if (e.key === 'ArrowRight') { e.preventDefault(); pause(); stepForward(); }
  if (e.key === 'ArrowLeft') { e.preventDefault(); pause(); undoOne(); }
  if (e.key === 'Backspace') { e.preventDefault(); pause(); undoOne(); }
});

function buildManualButtons() {
  const moves = [
    'R', "R'", 'R2',
    'L', "L'", 'L2',
    'U', "U'", 'U2',
    'D', "D'", 'D2',
    'F', "F'", 'F2',
    'B', "B'", 'B2',
  ];
  elManualGrid.innerHTML = '';
  for (const m of moves) {
    const b = document.createElement('button');
    b.type = 'button';
    b.textContent = m;
    b.title = `Apply ${m}`;
    b.className = 'rounded-xl border border-white/10 bg-white/5 px-2 py-2 font-mono text-xs font-semibold hover:bg-white/10';
    b.addEventListener('click', () => { pause(); applyManualMove(m); });
    elManualGrid.appendChild(b);
  }
}

// init
resetView();
controls.saveState();
buildManualButtons();
resetCube();
loadSequence();

// resizable split
{
  const KEY = 'rubik.leftW';
  const clamp = (v, lo, hi) => Math.max(lo, Math.min(hi, v));

  function applyW(w) {
    elLeftPanel.style.width = `${Math.round(w)}px`;
  }

  const saved = Number(localStorage.getItem(KEY));
  if (Number.isFinite(saved) && saved > 0) applyW(saved);

  let drag = null;
  elSplitter.addEventListener('pointerdown', (e) => {
    if (window.matchMedia && window.matchMedia('(max-width: 768px)').matches) return;
    elSplitter.setPointerCapture(e.pointerId);
    const base = document.body.getBoundingClientRect();
    const current = elLeftPanel.getBoundingClientRect().width || 360;
    drag = { startX: e.clientX, startW: current, baseW: base.width };
    e.preventDefault();
  });

  elSplitter.addEventListener('pointermove', (e) => {
    if (!drag) return;
    const dx = e.clientX - drag.startX;
    const minW = 260;
    const maxW = clamp(drag.baseW - 360, 320, 720);
    const next = clamp(drag.startW + dx, minW, maxW);
    applyW(next);
    e.preventDefault();
  });

  window.addEventListener('pointerup', () => {
    if (!drag) return;
    const w = elLeftPanel.getBoundingClientRect().width || 360;
    localStorage.setItem(KEY, String(Math.round(w)));
    drag = null;
  });
}
