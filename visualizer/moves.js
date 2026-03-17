export const VALID_MOVES = new Set([
  'R', 'L', 'U', 'D', 'F', 'B',
  "R'", "L'", "U'", "D'", "F'", "B'",
  'R2', 'L2', 'U2', 'D2', 'F2', 'B2',
]);

export const MOVE_DEF = {
  R: { axis: 'x', layer: 1, baseDir: 1 },
  L: { axis: 'x', layer: -1, baseDir: -1 },
  U: { axis: 'y', layer: 1, baseDir: 1 },
  D: { axis: 'y', layer: -1, baseDir: -1 },
  F: { axis: 'z', layer: 1, baseDir: -1 },
  B: { axis: 'z', layer: -1, baseDir: 1 },
};

export function parseMoves(raw) {
  const tokens = String(raw || '').trim().split(/\s+/).filter(Boolean);
  const invalid = tokens.filter(t => !VALID_MOVES.has(t));
  if (invalid.length) throw new Error(`Invalid move(s): ${invalid.join(', ')}`);
  return tokens;
}

export function invertMove(m) {
  if (m.endsWith('2')) return m;
  return m.endsWith("'") ? m.slice(0, -1) : `${m}'`;
}
