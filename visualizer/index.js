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
  R: { axis: 'x', layer: 1,  baseDir: 1 },
  L: { axis: 'x', layer: -1, baseDir: -1 },
  U: { axis: 'y', layer: 1,  baseDir: 1 },
  D: { axis: 'y', layer: -1, baseDir: -1 },
  F: { axis: 'z', layer: 1,  baseDir: -1 },
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

const btnLoad = document.getElementById('btnLoad');
const btnResetCube = document.getElementById('btnResetCube');
const btnResetView = document.getElementById('btnResetView');
const btnBack = document.getElementById('btnBack');
const btnStep = document.getElementById('btnStep');
const btnPlay = document.getElementById('btnPlay');
const btnPause = document.getElementById('btnPause');
const elSpeed = document.getElementById('speed');
const elSpeedLabel = document.getElementById('speedLabel');
const elManualMove = document.getElementById('manualMove');
const btnManual = document.getElementById('btnManual');

const state = {
  sequence: [],
  seqIndex: 0,
  playing: false,
  busy: false,
  playTimer: null,
  appliedHistory: [], // actual moves applied in the current cube state (including manual)
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

// --- THREE scene
const renderer = new THREE.WebGLRenderer({ canvas, antialias: true, alpha: true });
renderer.setPixelRatio(Math.min(2, window.devicePixelRatio || 1));
renderer.outputColorSpace = THREE.SRGBColorSpace;

const scene = new THREE.Scene();
scene.fog = new THREE.Fog(0x07080b, 8, 24);

const camera = new THREE.PerspectiveCamera(38, 1, 0.1, 100);
camera.position.set(7.5, 6.0, 7.5);

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

function animateFrame() {
  controls.update();
  renderer.render(scene, camera);
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
    .finally(() => { state.busy = false; updateButtons(); });
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

function resetCube() {
  state.sequence = [];
  state.seqIndex = 0;
  state.playing = false;
  state.appliedHistory = [];
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

  btnLoad.disabled = state.busy;
  btnResetCube.disabled = state.busy;
  btnResetView.disabled = state.busy;
  btnManual.disabled = state.busy;
  btnStep.disabled = !canStepFwd;
  btnBack.disabled = !canUndo;
  btnPlay.disabled = state.busy || state.sequence.length === 0;
  btnPause.disabled = state.busy || !state.playing;
}

async function stepForward() {
  if (state.busy) return;
  if (state.seqIndex >= state.sequence.length) return;
  const m = state.sequence[state.seqIndex];
  setStatus(`Step: ${m}`);
  setError('');
  await applyMoveAnimated(m);
  state.appliedHistory.push(m);
  state.seqIndex++;
  updateHUD();
  updateButtons();
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
  state.sequence = [...scramble, ...solve];
  state.seqIndex = 0;
  setStatus(`Loaded: ${scramble.length} scramble + ${solve.length} solve`);
  updateHUD();
  updateButtons();
}

async function applyManual() {
  if (state.busy) return;
  const m = String(elManualMove.value || '').trim();
  if (!VALID_MOVES.has(m)) { setError(`Invalid move: ${m}`); return; }
  setStatus(`Manual: ${m}`);
  setError('');
  await applyMoveAnimated(m);
  state.appliedHistory.push(m);
  updateHUD();
  updateButtons();
}

btnLoad.addEventListener('click', loadSequence);
btnResetCube.addEventListener('click', resetCube);
btnResetView.addEventListener('click', resetView);
btnStep.addEventListener('click', () => { pause(); stepForward(); });
btnBack.addEventListener('click', () => { pause(); undoOne(); });
btnPlay.addEventListener('click', play);
btnPause.addEventListener('click', pause);
btnManual.addEventListener('click', () => { pause(); applyManual(); });

document.addEventListener('keydown', (e) => {
  const tag = (e.target && e.target.tagName) ? e.target.tagName.toUpperCase() : '';
  if (tag === 'INPUT' || tag === 'TEXTAREA' || tag === 'SELECT') return;
  if (e.key === ' ') { e.preventDefault(); state.playing ? pause() : play(); }
  if (e.key === 'ArrowRight') { e.preventDefault(); pause(); stepForward(); }
  if (e.key === 'ArrowLeft') { e.preventDefault(); pause(); undoOne(); }
  if (e.key === 'Backspace') { e.preventDefault(); pause(); undoOne(); }
});

// init
resetView();
controls.saveState();
resetCube();
loadSequence();
