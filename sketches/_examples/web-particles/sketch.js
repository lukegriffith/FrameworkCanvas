// Example: seeded particle swarm, seamless loop, stateless. (Port of _examples/particles.)
//
// Pattern to copy: every particle's position is a CLOSED-FORM function of ctx.progress,
// not an integrated simulation. That makes the loop perfect (progress 1.0 == 0.0), makes
// any frame renderable on its own (make frame T=...), and makes trails free: just
// evaluate earlier progress values.

let ps = [];

function setup() {
	createCanvas(ctx.w, ctx.h);
	colorMode(HSB, 255);
	noStroke();
	// random() is seeded by the harness (SEED), so this is reproducible.
	for (let i = 0; i < 500; i++) {
		const radius = random(0.08, 0.48);
		ps.push({
			angle0: random(TWO_PI),
			radius,
			spin: (random() < 0.5 ? 1 : -1) * floor(random(1, 4)),   // whole turns per loop => seamless
			wobble: random(0.05, 0.25),
			wobbleK: floor(random(1, 4)),
			phase: random(),
			hue: map(radius, 0.08, 0.48, 120, 230),                  // teal -> blue
			size: random(0.002, 0.006),
		});
	}
}

function pos(p, prog, unit) {
	const a = p.angle0 + p.spin * TWO_PI * prog;
	const r = p.radius * (1 + p.wobble * sin(TWO_PI * (p.wobbleK * prog + p.phase)));
	return [cos(a) * r * unit, sin(a) * r * unit];
}

function draw() {
	const unit = min(width, height);
	background('#080a14');                    // hex string: exact whatever colorMode is active
	blendMode(ADD);                            // overlapping glow
	translate(width / 2, height / 2);

	const tail = 10;
	const tailStep = 0.004;                    // progress units between tail samples
	for (const p of ps) {
		for (let j = 0; j < tail; j++) {
			const fade = 1 - j / tail;
			fill(p.hue, 200, 255, 90 * fade);
			const [x, y] = pos(p, ctx.progress - j * tailStep, unit);   // negative progress is fine: it's sin/cos
			circle(x, y, 2 * p.size * unit * (0.4 + 0.6 * fade));
		}
	}
	blendMode(BLEND);
}
