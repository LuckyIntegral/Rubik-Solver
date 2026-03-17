#!/usr/bin/env node
'use strict';

const http = require('http');
const fs = require('fs');
const path = require('path');

const PORT = Number(process.env.PORT || 3000);
const INDEX_HTML_PATH = path.resolve(__dirname, 'visualizer', 'index.html');
const INDEX_JS_PATH = path.resolve(__dirname, 'visualizer', 'index.js');
const APP_JS_PATH = path.resolve(__dirname, 'visualizer', 'app.js');
const SCENE_JS_PATH = path.resolve(__dirname, 'visualizer', 'scene.js');
const MOVES_JS_PATH = path.resolve(__dirname, 'visualizer', 'moves.js');

const INDEX_HTML = fs.readFileSync(INDEX_HTML_PATH);
const INDEX_JS = fs.readFileSync(INDEX_JS_PATH);
const APP_JS = fs.readFileSync(APP_JS_PATH);
const SCENE_JS = fs.readFileSync(SCENE_JS_PATH);
const MOVES_JS = fs.readFileSync(MOVES_JS_PATH);

http
  .createServer((req, res) => {
    const urlPath = (req.url || '/').split('?')[0];

    if (req.method !== 'GET') {
      res.writeHead(405, { 'Content-Type': 'text/plain; charset=utf-8' });
      return res.end('Method Not Allowed');
    }

    if (urlPath === '/' || urlPath === '/index.html') {
      res.writeHead(200, { 'Content-Type': 'text/html; charset=utf-8', 'Cache-Control': 'no-store' });
      return res.end(INDEX_HTML);
    }

    if (urlPath === '/index.js') {
      res.writeHead(200, { 'Content-Type': 'text/javascript; charset=utf-8', 'Cache-Control': 'no-store' });
      return res.end(INDEX_JS);
    }

    if (urlPath === '/app.js') {
      res.writeHead(200, { 'Content-Type': 'text/javascript; charset=utf-8', 'Cache-Control': 'no-store' });
      return res.end(APP_JS);
    }

    if (urlPath === '/scene.js') {
      res.writeHead(200, { 'Content-Type': 'text/javascript; charset=utf-8', 'Cache-Control': 'no-store' });
      return res.end(SCENE_JS);
    }

    if (urlPath === '/moves.js') {
      res.writeHead(200, { 'Content-Type': 'text/javascript; charset=utf-8', 'Cache-Control': 'no-store' });
      return res.end(MOVES_JS);
    }

    res.writeHead(404, { 'Content-Type': 'text/plain; charset=utf-8' });
    return res.end('Not Found');
  })
  .listen(PORT, '0.0.0.0', () => {
    console.log(`Rubik Visualizer  →  http://localhost:${PORT}`);
    console.log('Press Ctrl+C to stop.');
  });
