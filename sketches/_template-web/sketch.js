// Web (p5.js) sketch template. Copy this directory to sketches/<name>/ and edit.
//
//   cp -r sketches/_template-web sketches/mything
//   make check SKETCH=mything     # syntax check + render frame 0, prints OK or the error
//   make sheet SKETCH=mything     # look at the whole animation as one image
//
// This is ordinary global-mode p5.js (p5 1.11). The harness adds a read-only global `ctx`
// and makes everything deterministic. Rules:
//  - Animate from ctx.t (seconds) or ctx.progress (0..1 over the clip). NEVER use
//    frameCount, millis() as a free-running clock, Date.now() or performance.now().
//    (millis() is already derived from ctx.t, but ctx.t is clearer.)
//  - progress wraps, so sin(ctx.progress * TWO_PI * k) with whole-number k loops seamlessly.
//  - Size everything from width/height (== ctx.w/ctx.h). createCanvas() size arguments are
//    IGNORED: the harness always makes the canvas the requested output size.
//  - random() and noise() and Math.random() are all seeded from SEED, so they are reproducible.
//    Generate random data in setup(), not draw(), so it does not flicker.
//  - Load assets (images, fonts) in preload(), from paths relative to THIS directory.
//  - pixelDensity is fixed at 1. noLoop/loop/frameRate calls are harmless no-ops.
//  - Unlike the C++ lane, p5 does NOT clear the canvas between frames. Call background()
//    at the top of draw() unless you want trails (see docs/P5_NOTES.md on trails).

function setup() {
	createCanvas(ctx.w, ctx.h);
	// One-time setup: generate random data, load fonts, set colorMode, etc.
}

function draw() {
	background(0);

	const unit = min(width, height);            // scale-independent length
	noStroke();
	fill(255);
	circle(width / 2, height / 2, unit * (0.4 + 0.1 * sin(ctx.progress * TWO_PI)));
}
