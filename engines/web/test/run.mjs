// Web engine tests. Run with `make test-web`. Plain node, no framework.
import { spawnSync } from 'node:child_process';
import fs from 'node:fs';
import os from 'node:os';
import path from 'node:path';
import { fileURLToPath } from 'node:url';

const HERE = path.dirname(fileURLToPath(import.meta.url));
const RENDER = path.join(HERE, '..', 'render.mjs');
const tmp = fs.mkdtempSync(path.join(os.tmpdir(), 'fc-web-test-'));
let failed = 0;

function sketch(name, code) {
	const dir = path.join(tmp, name);
	fs.mkdirSync(dir, { recursive: true });
	fs.writeFileSync(path.join(dir, 'sketch.js'), code);
	return path.join(dir, 'sketch.js');
}
function render(sk, out, extra = []) {
	return spawnSync('node', [RENDER, '--sketch', sk, '--frames-dir', out, '--w', '64', '--h', '64', '--fps', '10', '--duration', '2', ...extra], { encoding: 'utf8' });
}
function check(name, ok, detail = '') {
	console.log(`${ok ? 'PASS' : 'FAIL'}  ${name}${ok ? '' : '  ' + detail}`);
	if (!ok) failed++;
}
const bytes = (f) => fs.readFileSync(f);

// Uses every source of randomness a model is likely to reach for.
const randomSketch = sketch('rand', `
function setup(){ createCanvas(10,10); }
function draw(){
  background(0); noStroke();
  for (let i = 0; i < 20; i++) {
    fill(random(255), Math.random() * 255, noise(i * 0.3, ctx.t) * 255);
    circle(random(width), Math.random() * height, 6 + random(8));
  }
}`);

{
	const a = path.join(tmp, 'a'), b = path.join(tmp, 'b'), c = path.join(tmp, 'c');
	const ra = render(randomSketch, a, ['--seed', '7', '--start', '10', '--end', '12']);
	const rb = render(randomSketch, b, ['--seed', '7', '--start', '10', '--end', '12']);
	const rc = render(randomSketch, c, ['--seed', '8', '--start', '10', '--end', '12']);
	check('renders succeed', ra.status === 0 && rb.status === 0 && rc.status === 0, ra.stderr + rb.stderr + rc.stderr);
	const names = fs.existsSync(a) ? fs.readdirSync(a).sort() : [];
	check('--start/--end writes exactly frames 10..12', JSON.stringify(names) === JSON.stringify(['frame_000010.png', 'frame_000011.png', 'frame_000012.png']), names.join(','));
	const same = names.length > 0 && names.every((n) => bytes(path.join(a, n)).equals(bytes(path.join(b, n))));
	check('same seed gives byte-identical frames', same);
	const differ = names.length > 0 && names.some((n) => !bytes(path.join(a, n)).equals(bytes(path.join(c, n))));
	check('different seed gives different frames', differ);
}

{
	// ctx values and millis() are derived from the frame index, never a clock.
	const sk = sketch('ctxvals', `
function setup(){ createCanvas(10,10); }
function draw(){ if (ctx.frame === 5) console.log('CTX ' + JSON.stringify({t: ctx.t, p: ctx.progress, f: ctx.frame, ms: millis(), w: ctx.w, h: ctx.h, fps: ctx.fps, d: ctx.duration, rec: ctx.recording, seed: ctx.seed})); }`);
	const r = render(sk, path.join(tmp, 'ctx'), ['--seed', '3']);
	const line = (r.stdout.match(/CTX (\{.*\})/) || [])[1];
	const v = line ? JSON.parse(line) : {};
	const ok = v.t === 0.5 && v.p === 0.25 && v.f === 5 && v.ms === 500 && v.w === 64 && v.h === 64 && v.fps === 10 && v.d === 2 && v.rec === true && v.seed === 3;
	check('ctx fields and millis() come from the frame index', ok, line || r.stderr);
}

{
	// frameCount is pinned to frame + 1 (p5's convention) wherever rendering starts.
	const sk = sketch('fc', `function setup(){ createCanvas(10,10); } function draw(){ if (ctx.frame === 6) console.log('FC ' + frameCount); }`);
	const r0 = render(sk, path.join(tmp, 'fc0'), ['--start', '0', '--end', '6']);
	const r1 = render(sk, path.join(tmp, 'fc1'), ['--start', '6', '--end', '6']);
	check('frameCount is frame + 1 from any start frame', /FC 7/.test(r0.stdout) && /FC 7/.test(r1.stdout), r0.stdout + r1.stdout);
}

{
	// Hard-coded createCanvas size and pixelDensity are overridden to the requested output size.
	const sk = sketch('size', `function setup(){ createCanvas(800, 600); pixelDensity(2); } function draw(){ background(200); }`);
	const out = path.join(tmp, 'size');
	const r = render(sk, out, ['--start', '0', '--end', '0']);
	const png = fs.existsSync(path.join(out, 'frame_000000.png')) ? bytes(path.join(out, 'frame_000000.png')) : Buffer.alloc(0);
	const w = png.readUInt32BE(16), h = png.readUInt32BE(20);   // IHDR width/height
	check('canvas is forced to the output size', r.status === 0 && w === 64 && h === 64, `got ${w}x${h} ${r.stderr}`);
}

{
	// params reach the sketch.
	const sk = sketch('params', `function setup(){ createCanvas(10,10); } function draw(){ if (ctx.frame === 0) console.log('PARAM ' + ctx.params.answer); }`);
	const pf = path.join(tmp, 'p.json'); fs.writeFileSync(pf, '{"answer": 42}');
	const r = render(sk, path.join(tmp, 'pp'), ['--params', pf, '--start', '0', '--end', '0']);
	check('--params populates ctx.params', /PARAM 42/.test(r.stdout), r.stdout + r.stderr);
}

{
	const bad = sketch('bad', `function setup(){ createCanvas(10,10); }\nfunction draw(){\n  undefinedThing();\n}`);
	const r = render(bad, path.join(tmp, 'bad'));
	check('runtime error exits 1 and names sketch.js line', r.status === 1 && /sketch\.js:3/.test(r.stderr), r.stderr);
	const nodraw = sketch('nodraw', `function setup(){ createCanvas(10,10); }`);
	const r2 = render(nodraw, path.join(tmp, 'nodraw'));
	check('missing draw() exits 1 with a clear message', r2.status === 1 && /must define function draw/.test(r2.stderr), r2.stderr);
	const r3 = spawnSync('node', [RENDER, '--bogus', '1'], { encoding: 'utf8' });
	check('bad arguments exit 2', r3.status === 2, r3.stderr);
}

{
	// Every js block in docs/P5_NOTES.md must actually run. `js` blocks are draw() fragments,
	// `js sketch` blocks are complete sketches.
	const doc = fs.readFileSync(path.join(HERE, '..', '..', '..', 'docs', 'P5_NOTES.md'), 'utf8');
	const blocks = [...doc.matchAll(/```(js sketch|js)\n([\s\S]*?)```/g)].map((m) => ({ full: m[1] === 'js sketch', code: m[2] }));
	// Tiny PNG for the asset example: 1x1 red.
	const dotPng = Buffer.from('iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAYAAAAfFcSJAAAADUlEQVR4nGP8z8DwHwAFAAH/q842iQAAAABJRU5ErkJggg==', 'base64');
	let n = 0, bad = [];
	for (const b of blocks) {
		const code = b.full ? b.code : `function setup(){ createCanvas(ctx.w, ctx.h); }\nfunction draw(){\n${b.code}\n}`;
		const sk = sketch('doc' + n, code);
		fs.writeFileSync(path.join(path.dirname(sk), 'dot.png'), dotPng);
		const r = render(sk, path.join(tmp, 'docout' + n), ['--start', '5', '--end', '5']);
		if (r.status !== 0) bad.push(`block ${n}: ${r.stderr.trim().split('\n')[0]}`);
		n++;
	}
	check(`all ${blocks.length} snippets in docs/P5_NOTES.md run`, blocks.length >= 8 && bad.length === 0, bad.join(' | '));
}

fs.rmSync(tmp, { recursive: true, force: true });
console.log(failed ? `\n${failed} check(s) failed` : '\nall web engine checks passed');
process.exit(failed ? 1 : 0);
