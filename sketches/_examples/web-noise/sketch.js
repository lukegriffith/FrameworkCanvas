// Example: flow-field streamlines through looping noise. (Port of _examples/noise-field.)
//
// Pattern to copy: p5's noise() takes at most THREE inputs (the C++ lane has four), so the
// loop trick is different. Slide the sample point around a CIRCLE in x,y as progress goes
// 0..1: the field then returns exactly to its start, so the loop is seamless with no
// cross-fade. Streamlines are re-traced every frame, so nothing is stored between frames.

let seeds = [];
let inks = [];

function setup() {
	createCanvas(ctx.w, ctx.h);
	noFill();
	for (let i = 0; i < 1800; i++) {
		seeds.push([random(-0.05, 1.05), random(-0.05, 1.05)]);   // fractions of the canvas
		inks.push(random() < 0.8 ? [24, 38, 82, 150] : [200, 60, 40, 190]);
	}
}

// Angle of the flow at (x, y) in unit space.
function flowAngle(x, y, progress) {
	const a = progress * TWO_PI;
	const loopR = 0.5;                                  // bigger = more change over the loop
	const n = noise(x * 1.6 + 10 + cos(a) * loopR, y * 1.6 + 10 + sin(a) * loopR);
	return n * TWO_PI * 2.5;
}

function draw() {
	background(244, 238, 226);                          // paper
	// "unit space": 1.0 = the short side, so lengths and noise frequency look the same in
	// square, portrait and wide formats while seeds still cover the whole canvas.
	const unit = min(width, height);
	strokeWeight(max(1, unit / 700));

	const steps = 36, stepLen = 0.006;
	for (let i = 0; i < seeds.length; i++) {
		let px = seeds[i][0] * width / unit;
		let py = seeds[i][1] * height / unit;
		stroke(...inks[i]);
		beginShape();
		for (let s = 0; s < steps; s++) {
			vertex(px * unit, py * unit);
			const a = flowAngle(px, py, ctx.progress);
			px += cos(a) * stepLen;
			py += sin(a) * stepLen;
		}
		endShape();
	}
}
