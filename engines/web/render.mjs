#!/usr/bin/env node
// HARNESS (node side). Renders a p5.js sketch to numbered PNG frames in headless Chromium.
// Not for sketch authors. Interface mirrors the OpenFrameworks binary's env vars:
//
//   node render.mjs --sketch sketches/x/sketch.js --frames-dir out/x/frames \
//        --w 1920 --h 1080 --fps 30 --duration 10 [--start 0] [--end N] [--seed 1] [--params file.json]
//
// Exit codes: 0 ok, 1 sketch error (message printed), 2 bad arguments, 3 internal/driver error.

import http from 'node:http';
import fs from 'node:fs';
import path from 'node:path';
import { fileURLToPath } from 'node:url';

const HERE = path.dirname(fileURLToPath(import.meta.url));

class UsageError extends Error {}
class SketchError extends Error {}

function parseArgs(argv) {
	const a = { w: 1920, h: 1080, fps: 30, duration: 10, start: 0, end: -1, seed: 1, timeout: 15000, frameTimeout: 60000 };
	const ints = new Set(['w', 'h', 'fps', 'duration', 'start', 'end', 'seed', 'timeout', 'frame-timeout']);
	const strs = new Set(['sketch', 'frames-dir', 'params']);
	for (let i = 0; i < argv.length; i++) {
		const k = argv[i].replace(/^--/, '');
		if (!argv[i].startsWith('--')) throw new UsageError(`unexpected argument '${argv[i]}'`);
		const v = argv[++i];
		if (v === undefined) throw new UsageError(`missing value for --${k}`);
		if (ints.has(k)) {
			const n = Number(v);
			if (!Number.isFinite(n)) throw new UsageError(`--${k} must be a number, got '${v}'`);
			a[k === 'frame-timeout' ? 'frameTimeout' : k] = Math.trunc(n);
		} else if (strs.has(k)) a[k === 'frames-dir' ? 'framesDir' : k] = v;
		else throw new UsageError(`unknown option --${k}`);
	}
	if (!a.sketch) throw new UsageError('--sketch is required');
	if (!a.framesDir) throw new UsageError('--frames-dir is required');
	for (const k of ['w', 'h', 'fps', 'duration']) if (a[k] < 1) throw new UsageError(`--${k} must be >= 1`);
	const total = a.fps * a.duration;
	if (a.end < 0 || a.end >= total) a.end = total - 1;
	if (a.start < 0 || a.start > a.end) throw new UsageError(`--start ${a.start} is outside 0..${a.end}`);
	a.sketch = path.resolve(a.sketch);
	a.framesDir = path.resolve(a.framesDir);
	if (!fs.existsSync(a.sketch)) throw new UsageError(`sketch not found: ${a.sketch}`);
	if (a.params) {
		try { a.paramsData = JSON.parse(fs.readFileSync(path.resolve(a.params), 'utf8')); }
		catch (e) { throw new UsageError(`cannot read --params ${a.params}: ${e.message}`); }
	} else a.paramsData = {};
	return a;
}

const MIME = { '.html': 'text/html', '.js': 'text/javascript', '.json': 'application/json', '.png': 'image/png',
	'.jpg': 'image/jpeg', '.jpeg': 'image/jpeg', '.gif': 'image/gif', '.svg': 'image/svg+xml',
	'.ttf': 'font/ttf', '.otf': 'font/otf', '.woff': 'font/woff', '.woff2': 'font/woff2', '.txt': 'text/plain', '.csv': 'text/csv' };

function shellHtml() {
	return `<!doctype html><html><head><meta charset="utf-8">
<style>html,body{margin:0;padding:0;background:#000;overflow:hidden}</style>
<script src="/harness.js"></script>
<script src="/p5.js"></script>
<script src="/sketch.js"></script>
<script src="/harness-post.js"></script>
</head><body></body></html>`;
}

// Serves the page, p5, the harness, the sketch, and any other file from the sketch's
// directory (so loadImage('photo.jpg') / loadFont('x.ttf') resolve relative to the sketch).
function startServer(args) {
	const sketchDir = path.dirname(args.sketch);
	const p5Path = path.join(HERE, 'node_modules', 'p5', 'lib', 'p5.min.js');
	const fixed = {
		'/': () => ['text/html', shellHtml()],
		'/harness.js': () => ['text/javascript', fs.readFileSync(path.join(HERE, 'harness.js'))],
		'/harness-post.js': () => ['text/javascript', fs.readFileSync(path.join(HERE, 'harness-post.js'))],
		'/p5.js': () => ['text/javascript', fs.readFileSync(p5Path)],
		'/sketch.js': () => ['text/javascript', fs.readFileSync(args.sketch)],
	};
	const server = http.createServer((req, res) => {
		try {
			const url = decodeURIComponent(new URL(req.url, 'http://x').pathname);
			if (url === '/favicon.ico') { res.writeHead(204); return res.end(); }
			if (fixed[url]) {
				const [type, body] = fixed[url]();
				res.writeHead(200, { 'Content-Type': type, 'Cache-Control': 'no-store' });
				return res.end(body);
			}
			const file = path.resolve(sketchDir, '.' + url);
			if (!file.startsWith(sketchDir + path.sep) || !fs.statSync(file).isFile()) throw new Error('nf');
			res.writeHead(200, { 'Content-Type': MIME[path.extname(file)] || 'application/octet-stream' });
			res.end(fs.readFileSync(file));
		} catch {
			res.writeHead(404); res.end('not found');
		}
	});
	return new Promise((resolve) => server.listen(0, '127.0.0.1', () => resolve(server)));
}

function withTimeout(promise, ms, message) {
	let timer;
	const limit = new Promise((_, rej) => { timer = setTimeout(() => rej(new SketchError(message)), ms); });
	return Promise.race([promise, limit]).finally(() => clearTimeout(timer));
}

// Turn a browser error into a short message that points at sketch.js lines.
function tidy(text) {
	const lines = String(text).replace(/http:\/\/127\.0\.0\.1:\d+\//g, '').split('\n');
	const out = [lines[0].replace(/^Evaluation failed: /, '')];
	for (const l of lines.slice(1)) if (/sketch\.js:\d+/.test(l)) out.push(l.trim());
	return out.slice(0, 4).join('\n    ');
}

async function main() {
	const args = parseArgs(process.argv.slice(2));
	if (!fs.existsSync(path.join(HERE, 'node_modules', 'p5', 'lib', 'p5.min.js')))
		throw new UsageError('web engine not installed. Run: make setup-web');
	const { default: puppeteer } = await import('puppeteer');

	fs.mkdirSync(args.framesDir, { recursive: true });
	const server = await startServer(args);
	const port = server.address().port;
	const browser = await puppeteer.launch({ headless: true, args: ['--hide-scrollbars', '--mute-audio', '--enable-unsafe-swiftshader'] });

	let rejectFailure;
	const failure = new Promise((_, rej) => { rejectFailure = rej; });
	failure.catch(() => {});   // handled via race below
	const fail = (msg) => rejectFailure(new SketchError(msg));
	let logged = 0;

	try {
		const page = await browser.newPage();
		await page.setViewport({ width: args.w, height: args.h, deviceScaleFactor: 1 });
		page.on('pageerror', (e) => fail(tidy(e.stack || e.message || e)));
		page.on('response', (r) => {
			if (r.status() >= 400) fail(`missing file: ${r.url().replace(/^http:\/\/127\.0\.0\.1:\d+/, '')} (${r.status()}). Assets load relative to the sketch's directory`);
		});
		page.on('console', (m) => {
			if (/Failed to load resource/.test(m.text())) return;   // reported by the response handler
			if (m.type() === 'error') fail('console.error: ' + tidy(m.text()));
			else if (logged++ < 40) console.log('[sketch console]', m.text());
		});
		await page.evaluateOnNewDocument((cfg) => { window.__FC__ = cfg; }, {
			w: args.w, h: args.h, fps: args.fps, duration: args.duration,
			seed: args.seed, start: args.start, params: args.paramsData,
		});

		const guard = (p) => Promise.race([p, failure]);
		const stuck = `setup() did not finish within ${args.timeout} ms (infinite loop, or a missing/slow asset?)`;
		try {
			// p5 runs the sketch's setup() inside the window 'load' event, so a setup() that never
			// returns blocks the page; both the navigation and the readiness wait are bounded.
			await guard(page.goto(`http://127.0.0.1:${port}/`, { waitUntil: 'domcontentloaded', timeout: args.timeout }));
			const err = await withTimeout(guard(page.evaluate(() => window.__error || null)), args.timeout, stuck);
			if (err) throw new SketchError(err);
			await withTimeout(guard(page.waitForFunction('window.__ready === true', { timeout: args.timeout })), args.timeout + 1000, stuck);
		} catch (e) {
			if (e instanceof SketchError) throw e;
			throw new SketchError(stuck);
		}

		for (let f = args.start; f <= args.end; f++) {
			const hung = `draw() did not return within ${args.frameTimeout} ms at frame ${f} (infinite loop?)`;
			let stack;
			try {
				stack = await withTimeout(guard(page.evaluate((n) => window.__step(n), f)), args.frameTimeout, hung);
			} catch (e) {
				if (e instanceof SketchError) throw e;
				throw new SketchError(`draw() failed at frame ${f}: ${tidy(e.message)}`);
			}
			if (stack) throw new SketchError(`draw() threw at frame ${f}: ${tidy(stack)}`);
			const data = await withTimeout(guard(page.evaluate(() => window.__capture())), args.frameTimeout, `capture timed out at frame ${f}`);
			fs.writeFileSync(path.join(args.framesDir, `frame_${String(f).padStart(6, '0')}.png`),
				Buffer.from(data.split(',')[1], 'base64'));
		}
		if (!process.env.FC_QUIET) console.log(`rendered ${args.end - args.start + 1} frames to ${args.framesDir}`);
	} finally {
		// A page blocked in an infinite loop can ignore a polite close; don't wait long, then kill.
		await Promise.race([browser.close().catch(() => {}), new Promise((r) => setTimeout(r, 3000))]);
		browser.process()?.kill('SIGKILL');
		server.close();
	}
}

main().then(() => process.exit(0), (e) => {
	if (e instanceof UsageError) { console.error('usage error: ' + e.message); process.exit(2); }
	if (e instanceof SketchError) { console.error('sketch error: ' + e.message); process.exit(1); }
	console.error('render driver error: ' + (e.stack || e)); process.exit(3);
});
