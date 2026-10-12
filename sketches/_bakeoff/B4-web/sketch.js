// B4 glow field: metaballs with a neon ramp. Field is computed at low res and upscaled (smooth).
let img, W, H, blobs = [];
const N = 6;

function setup() {
	createCanvas(ctx.w, ctx.h);
	W = 384; H = round(W * ctx.h / ctx.w);
	img = createImage(W, H);
	for (let i = 0; i < N; i++) {
		blobs.push({
			fx: 1 + floor(random(0, 3)), fy: 1 + floor(random(0, 3)),
			px: random(TWO_PI), py: random(TWO_PI),
			ax: random(0.22, 0.4), ay: random(0.2, 0.36),
			r: random(0.1, 0.17),
			fr: 1 + floor(random(0, 2)), pr: random(TWO_PI),
		});
	}
}

const stops = [
	[0.00, 8, 4, 30], [0.25, 40, 10, 110], [0.5, 190, 20, 180],
	[0.75, 255, 70, 140], [0.9, 255, 190, 120], [1.0, 255, 250, 235],
];
function ramp(v) {
	v = Math.min(1, Math.max(0, v));
	for (let i = 1; i < stops.length; i++) {
		if (v <= stops[i][0]) {
			const a = stops[i - 1], b = stops[i], k = (v - a[0]) / (b[0] - a[0]);
			const s = k * k * (3 - 2 * k);
			return [a[1] + (b[1] - a[1]) * s, a[2] + (b[2] - a[2]) * s, a[3] + (b[3] - a[3]) * s];
		}
	}
	return [255, 250, 235];
}
// cyan accent mixed in via a second channel
function draw() {
	const a = ctx.progress * TWO_PI;
	const asp = ctx.w / ctx.h;
	const bs = blobs.map(b => ({
		x: 0.5 * asp + Math.sin(a * b.fx + b.px) * b.ax * asp,
		y: 0.5 + Math.sin(a * b.fy + b.py) * b.ay,
		r: b.r * (1 + 0.18 * Math.sin(a * b.fr + b.pr)),
	}));
	img.loadPixels();
	const px = img.pixels;
	let o = 0;
	for (let j = 0; j < H; j++) {
		const y = j / H;
		for (let i = 0; i < W; i++) {
			const x = (i / W) * asp;
			let f = 0;
			for (let k = 0; k < N; k++) {
				const b = bs[k], dx = x - b.x, dy = y - b.y;
				f += (b.r * b.r) / (dx * dx + dy * dy + 1e-5);
			}
			const v = 1 - Math.exp(-f * 0.9);
			const c = ramp(v);
			// teal tint in the faint halo
			const halo = Math.max(0, 0.35 - v) * 2;
			px[o++] = c[0];
			px[o++] = c[1] + halo * 60;
			px[o++] = c[2] + halo * 80;
			px[o++] = 255;
		}
	}
	img.updatePixels();
	background(0);
	drawingContext.imageSmoothingEnabled = true;
	drawingContext.imageSmoothingQuality = 'high';
	image(img, 0, 0, width, height);
}
