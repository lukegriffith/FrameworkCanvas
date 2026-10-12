// Particle vortex: teal and orange particles spiralling round a dark centre. Stateless, loops.
const N = 420;
const TRAIL = 16;
let P = [];

function setup() {
	createCanvas(ctx.w, ctx.h);
	for (let i = 0; i < N; i++) {
		P.push({
			r: random(0.12, 0.46),        // base radius (fraction of unit)
			a0: random(TWO_PI),
			k: floor(random(1, 4)),       // whole turns per loop (inner ones are faster in feel via k)
			breathe: floor(random(1, 4)), // whole radial cycles per loop
			bph: random(TWO_PI),
			amp: random(0.01, 0.05),
			size: random(0.004, 0.011),
			warm: random() < 0.45,
			tw: random(TWO_PI),
		});
	}
}

function draw() {
	const u = min(width, height);
	const cx = width / 2, cy = height / 2;
	const A = ctx.progress * TWO_PI;
	background('#05080d');
	noStroke();
	const ctxd = drawingContext;
	ctxd.globalCompositeOperation = 'lighter';

	// faint dark-centre halo ring glow
	for (let g = 0; g < 6; g++) {
		fill(20, 90, 100, 6);
		circle(cx, cy, u * (0.3 + g * 0.1));
	}

	for (const p of P) {
		for (let j = TRAIL - 1; j >= 0; j--) {
			const back = j * 0.0045;                       // progress offset into the past
			const a = p.a0 + p.k * (A - back * TWO_PI) * (1 + 0 * j);
			const r = (p.r + p.amp * sin(p.breathe * (A - back * TWO_PI) + p.bph)) * u;
			// tighten inward spiral: squash radius so nothing enters the dark core
			const x = cx + cos(a) * r;
			const y = cy + sin(a) * r;
			const f = 1 - j / TRAIL;
			const tw = 0.75 + 0.25 * sin(p.tw + 2 * A);
			const al = 255 * f * f * tw;
			const s = u * p.size * (0.4 + 0.6 * f);
			if (p.warm) fill(255, 130 + 40 * f, 40, al * 0.55);
			else fill(30, 210, 190, al * 0.55);
			circle(x, y, s * 2.4);                          // soft halo
			if (j < 3) {
				fill(255, 245, 230, al * 0.5);
				circle(x, y, s * 0.8);                      // hot core on the head
			}
		}
	}
	ctxd.globalCompositeOperation = 'source-over';
}
