// B3: layered dusk ridgelines with parallax. Each ridge is a sum of integer harmonics over
// its own period, and drifts exactly one period per clip, so the loop is seamless.
const N = 9;
let layers = [];

function setup() {
	createCanvas(ctx.w, ctx.h);
	for (let i = 0; i < N; i++) {
		const d = i / (N - 1);                       // 0 = back, 1 = front
		const harm = [];
		for (let k = 1; k <= 5; k++) {
			harm.push({ k: k + (k > 2 ? 0 : 0), a: random(0.5, 1) / pow(k, 1.5), ph: random(TWO_PI) });
		}
		layers.push({
			d,
			period: width * (0.55 + 0.5 * d),        // front layers cover more ground per loop = faster
			base: height * (0.50 + 0.40 * d),
			amp: height * (0.09 + 0.05 * d),
			harm,
		});
	}
}

function ridgeY(L, x, shift) {
	const u = (x + shift) / L.period;
	let s = 0;
	for (const h of L.harm) s += h.a * sin(TWO_PI * h.k * u + h.ph);
	return L.base - L.amp * s;
}

function draw() {
	// sky: dusk gradient
	const top = color(58, 40, 96), mid = color(214, 98, 120), hor = color(255, 190, 120);
	noStroke();
	for (let y = 0; y < height; y += 4) {
		const v = y / (height * 0.7);
		const c = v < 0.55 ? lerpColor(top, mid, v / 0.55) : lerpColor(mid, hor, constrain((v - 0.55) / 0.45, 0, 1));
		fill(c);
		rect(0, y, width, 4);
	}
	// sun glow
	for (let r = 24; r > 0; r--) {
		fill(255, 220, 160, 6);
		circle(width * 0.5, height * 0.30, width * 0.14 * r * 0.2);
	}
	fill(255, 236, 190);
	circle(width * 0.5, height * 0.30, width * 0.1);

	const warm = color(236, 120, 110), cool = color(22, 34, 78);
	const haze = color(255, 190, 140);
	for (let i = 0; i < N; i++) {
		const L = layers[i];
		const shift = ctx.progress * L.period;       // exactly one period per loop
		const e = pow(L.d, 0.8);
		let c = lerpColor(warm, cool, e);
		fill(c);
		beginShape();
		vertex(0, height);
		for (let x = 0; x <= width + 6; x += 6) vertex(x, ridgeY(L, x, shift));
		vertex(width + 6, height);
		endShape(CLOSE);
		// atmospheric haze fading toward the next layer's colour
		const top0 = L.base - L.amp * 1.6;
		const hz = lerpColor(haze, cool, e);
		for (let g = 0; g < 14; g++) {
			hz.setAlpha(0);
			const a = 38 * (1 - g / 14) * (1 - L.d * 0.7);
			fill(red(hz), green(hz), blue(hz), a);
			beginShape();
			vertex(0, height);
			for (let x = 0; x <= width + 6; x += 12) vertex(x, ridgeY(L, x, shift) + g * height * 0.006 + 2);
			vertex(width + 6, height);
			endShape(CLOSE);
		}
	}
}
