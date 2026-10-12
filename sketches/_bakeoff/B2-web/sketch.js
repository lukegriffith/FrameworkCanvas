// SIGNAL: kinetic typography, letters rise in a repeating wave. Loops seamlessly (2 whole cycles).
const WORD = 'SIGNAL';
const CYCLES = 2;

function setup() {
	createCanvas(ctx.w, ctx.h);
	textFont('Helvetica Neue');
	textStyle(BOLD);
	textAlign(LEFT, BASELINE);
	colorMode(HSB, 360, 100, 100, 100);
}

function draw() {
	const u = min(width, height);
	// background with a faint vertical gradient
	noStroke();
	for (let y = 0; y < height; y += 8) {
		const k = y / height;
		fill(235, 60, 7 + 6 * k);
		rect(0, y, width, 8);
	}

	const size = u * 0.2;
	textSize(size);
	const gap = u * 0.012;
	const widths = [...WORD].map(c => textWidth(c));
	const total = widths.reduce((a, b) => a + b, 0) + gap * (WORD.length - 1);
	let x = (width - total) / 2;
	const baseY = height / 2 + size * 0.35;
	const a = ctx.progress * TWO_PI * CYCLES;

	for (let i = 0; i < WORD.length; i++) {
		const ph = a - i * 0.75;
		const s = sin(ph);
		const lift = pow(max(0, s), 1.5);          // rises, then settles for the rest of the cycle
		const y = baseY - lift * u * 0.2;
		const hue = (200 + 140 * (0.5 + 0.5 * sin(ph - 0.4))) % 360;
		const sat = 55 + 35 * lift;
		const bri = 75 + 25 * lift;

		// glow + shadow on the baseline
		fill(hue, sat, bri, 12 + 25 * lift);
		for (let g = 3; g >= 1; g--) text(WORD[i], x - g * 2, y + g * 2);
		fill(0, 0, 0, 35 * (1 - lift));
		text(WORD[i], x + 6, baseY + 10);
		fill(hue, sat, bri);
		text(WORD[i], x, y);
		x += widths[i] + gap;
	}

	// settle line
	const ly = baseY + u * 0.05;
	for (let i = 0; i < WORD.length; i++) {
		fill(220, 30, 60, 40);
	}
	rect((width - total) / 2, ly, total, 4);
}
