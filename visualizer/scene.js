import * as THREE from 'https://esm.sh/three@0.161.0';
import { OrbitControls } from 'https://esm.sh/three@0.161.0/examples/jsm/controls/OrbitControls.js';
import { MOVE_DEF, VALID_MOVES } from './moves.js';

const FACE = {
  U: 0xFFFFFF,
  D: 0xFFD500,
  F: 0x009B48,
  B: 0x0046AD,
  L: 0xFF5900,
  R: 0xB71234,
};

const spacing = 1.05;
const snap90 = a => Math.round(a / (Math.PI / 2)) * (Math.PI / 2);
const snapCoord = (v) => {
  const n = Math.round(v);
  if (n > 1) return 1;
  if (n < -1) return -1;
  return n;
};

function axisVector(axis) {
  if (axis === 'x') return new THREE.Vector3(1, 0, 0);
  if (axis === 'y') return new THREE.Vector3(0, 1, 0);
  return new THREE.Vector3(0, 0, 1);
}

function makeStickerMaterial(color) {
  return new THREE.MeshBasicMaterial({ color });
}

const matBlank = new THREE.MeshBasicMaterial({ color: 0x101010 });
const mats = {
  U: makeStickerMaterial(FACE.U),
  D: makeStickerMaterial(FACE.D),
  F: makeStickerMaterial(FACE.F),
  B: makeStickerMaterial(FACE.B),
  L: makeStickerMaterial(FACE.L),
  R: makeStickerMaterial(FACE.R),
};

function materialsForCubie(x, y, z) {
  // BoxGeometry face order: +X, -X, +Y, -Y, +Z, -Z
  return [
    x === 1 ? mats.R : matBlank,
    x === -1 ? mats.L : matBlank,
    y === 1 ? mats.U : matBlank,
    y === -1 ? mats.D : matBlank,
    z === 1 ? mats.F : matBlank,
    z === -1 ? mats.B : matBlank,
  ];
}

function normalizeCubieTransform(c, root) {
  root.attach(c);
  const cx = snapCoord(c.position.x / spacing);
  const cy = snapCoord(c.position.y / spacing);
  const cz = snapCoord(c.position.z / spacing);
  c.position.set(cx * spacing, cy * spacing, cz * spacing);
  c.userData.coord = { x: cx, y: cy, z: cz };
  const e = new THREE.Euler().setFromQuaternion(c.quaternion, 'XYZ');
  e.set(snap90(e.x), snap90(e.y), snap90(e.z));
  c.quaternion.setFromEuler(e);
}

export function createScene({ canvas }) {
  const renderer = new THREE.WebGLRenderer({ canvas, antialias: true, alpha: true });
  renderer.setPixelRatio(Math.min(2, window.devicePixelRatio || 1));
  renderer.outputColorSpace = THREE.SRGBColorSpace;

  const scene = new THREE.Scene();

  const camera = new THREE.PerspectiveCamera(38, 1, 0.1, 100);
  camera.position.set(7.5, 6.0, 7.5);

  const controls = new OrbitControls(camera, canvas);
  controls.enableDamping = true;
  controls.dampingFactor = 0.075;
  controls.enablePan = false;
  controls.minDistance = 6.0;
  controls.maxDistance = 18.0;

  const cubeRoot = new THREE.Group();
  scene.add(cubeRoot);

  const cubies = [];
  const geom = new THREE.BoxGeometry(0.98, 0.98, 0.98);
  for (const x of [-1, 0, 1]) for (const y of [-1, 0, 1]) for (const z of [-1, 0, 1]) {
    const mesh = new THREE.Mesh(geom, materialsForCubie(x, y, z));
    mesh.position.set(x * spacing, y * spacing, z * spacing);
    mesh.userData.homeCoord = { x, y, z };
    mesh.userData.coord = { x, y, z };
    cubeRoot.add(mesh);
    cubies.push(mesh);
  }

  function resize() {
    const w = canvas.clientWidth, h = canvas.clientHeight;
    if (!w || !h) return;
    renderer.setSize(w, h, false);
    camera.aspect = w / h;
    camera.updateProjectionMatrix();
  }
  new ResizeObserver(resize).observe(canvas);
  resize();

  function renderLoop() {
    controls.update();
    renderer.render(scene, camera);
    requestAnimationFrame(renderLoop);
  }
  requestAnimationFrame(renderLoop);

  function resetView() {
    controls.reset();
    camera.position.set(7.5, 6.0, 7.5);
    controls.target.set(0, 0, 0);
    controls.update();
  }

  function resetCube() {
    for (const c of cubies) {
      const { x, y, z } = c.userData.homeCoord;
      c.position.set(x * spacing, y * spacing, z * spacing);
      c.rotation.set(0, 0, 0);
      c.userData.coord = { x, y, z };
    }
  }

  function cubiesInLayer(axis, layer) {
    return cubies.filter(c => c.userData.coord[axis] === layer);
  }

  async function applyMoveAnimated(move, durationMs) {
    if (!VALID_MOVES.has(move)) throw new Error(`Invalid move: ${move}`);
    const base = move[0];
    const turns = move.endsWith('2') ? 2 : 1;
    const { axis, layer, baseDir } = MOVE_DEF[base];
    const dir = move.endsWith("'") ? -baseDir : baseDir;
    const ax = axisVector(axis);

    const quarterTurn = () => new Promise(resolve => {
      const pivot = new THREE.Group();
      cubeRoot.add(pivot);
      const layerCubies = cubiesInLayer(axis, layer);
      for (const c of layerCubies) pivot.attach(c);

      const dur = Math.max(60, Number(durationMs) || 240);
      const start = performance.now();
      const target = (Math.PI / 2) * dir;

      const tick = (now) => {
        const t = Math.min(1, (now - start) / dur);
        const eased = t < 0.5 ? 4 * t * t * t : 1 - Math.pow(-2 * t + 2, 3) / 2;
        pivot.setRotationFromAxisAngle(ax, target * eased);
        if (t < 1) return requestAnimationFrame(tick);
        pivot.setRotationFromAxisAngle(ax, target);
        for (const c of layerCubies) normalizeCubieTransform(c, cubeRoot);
        cubeRoot.remove(pivot);
        resolve();
      };
      requestAnimationFrame(tick);
    });

    for (let i = 0; i < turns; i++) await quarterTurn();
  }

  controls.saveState();
  return { resetView, resetCube, applyMoveAnimated };
}
