#!/usr/bin/env node
'use strict';

const http = require('http');
const fs = require('fs');
const path = require('path');

const PORT = Number(process.env.PORT || 3000);
const ROOT = path.resolve(__dirname);
const VIS_ROOT = path.join(ROOT, 'visualizer');

const MIME = {
  '.html': 'text/html; charset=utf-8',
  '.js': 'application/javascript; charset=utf-8',
  '.css': 'text/css; charset=utf-8',
  '.ico': 'image/x-icon',
  '.png': 'image/png',
  '.jpg': 'image/jpeg',
  '.jpeg': 'image/jpeg',
  '.svg': 'image/svg+xml; charset=utf-8',
  '.json': 'application/json; charset=utf-8',
};

function safeJoin(root, urlPath) {
  const p = path.normalize(urlPath).replace(/^(\.\.(\/|\\|$))+/, '');
  const full = path.join(root, p);
  if (!full.startsWith(root)) return null;
  return full;
}

http
  .createServer((req, res) => {
    const rawUrl = req.url || '/';
    const urlPath = rawUrl.split('?')[0];

    // Default to the visualizer page.
    let mapped = urlPath;
    if (mapped === '/' || mapped === '/index.html') mapped = '/visualizer/index.html';

    // Allow both /visualizer/... and direct file hits inside visualizer.
    let filePath = null;
    if (mapped.startsWith('/visualizer/')) {
      filePath = safeJoin(VIS_ROOT, mapped.slice('/visualizer/'.length));
    } else {
      // if they request "/index.js", serve from visualizer for convenience
      filePath = safeJoin(VIS_ROOT, mapped.slice(1));
    }

    if (!filePath) {
      res.writeHead(403, { 'Content-Type': 'text/plain; charset=utf-8' });
      res.end('Forbidden');
      return;
    }

    fs.readFile(filePath, (err, data) => {
      if (err) {
        res.writeHead(404, { 'Content-Type': 'text/plain; charset=utf-8' });
        res.end(`Not found: ${urlPath}`);
        return;
      }
      const mime = MIME[path.extname(filePath).toLowerCase()] || 'application/octet-stream';
      res.writeHead(200, { 'Content-Type': mime, 'Cache-Control': 'no-store' });
      res.end(data);
    });
  })
  .listen(PORT, '0.0.0.0', () => {
    console.log(`Rubik Visualizer  →  http://localhost:${PORT}`);
    console.log('Press Ctrl+C to stop.');
  });

