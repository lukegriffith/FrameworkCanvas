# p5.js in FrameworkCanvas: harness notes

You already know p5. This page covers only what is different when a sketch runs under
the render harness (`sketches/<name>/sketch.js`, p5 **1.11**, global mode, headless
Chromium). Every `js` block below is executed by `make test-web`, so it stays true.

Fenced `js` blocks are fragments that are valid inside `draw()`. Blocks fenced
`js sketch` are complete sketches.

## What the harness does for you

| Behaviour | Detail |
|---|---|
| `ctx` global | Read-only: `w h t progress frame fps duration seed recording params`. `t = frame / fps`. |
| Size | `createCanvas(anything)` always produces `ctx.w x ctx.h`. Use `width`/`height`. |
| Pixel density | Fixed at 1. `pixelDensity(n)` is ignored. |
| Seeding | `random()`, `noise()` and `Math.random()` are all seeded from `SEED` (default 1). |
| `millis()` | Equals `ctx.t * 1000`. `Date.now()` and `performance.now()` are NOT patched: never use them. |
| `frameCount` | Equals `ctx.frame + 1` inside `draw()`, from any start frame. |
| `frameRate()` | Returns `ctx.fps`. `loop()` does nothing; p5's own loop never runs. |
| Drawing | `draw()` is called once per output frame by the driver, nothing else calls it. |
| Output | Canvas is composited on opaque black, so a transparent canvas exports as black. |

Not available: instance mode, p5.sound (`loadSound` is undefined), DOM elements,
video/webcam, mouse and keyboard events, `save()`/`saveCanvas()` (the harness saves
frames). p5 2.x APIs are not supported; the engine pins 1.11.

## Looping recipes

`ctx.progress` goes 0 to 1 over the clip and wraps. Anything periodic in it with a
**whole number** of cycles loops seamlessly.

```js
const a = ctx.progress * TWO_PI;
const pulse = sin(a * 3);                  // 3 cycles per clip
const ease = 0.5 - 0.5 * cos(a);           // 0 -> 1 -> 0, smooth at both ends
const pingpong = 1 - abs(2 * ctx.progress - 1);   // linear 0 -> 1 -> 0
background(20);
noStroke();
fill(255, 120, 60);
circle(width / 2 + pulse * width * 0.2, height / 2, height * (0.1 + 0.2 * ease));
```

`noise()` in p5 takes at most three inputs (the C++ lane's `ofNoise` takes four), so
loop it by sliding the sample point around a circle in x and y:

```js
const a = ctx.progress * TWO_PI;
const R = 0.6;                              // bigger radius = more change over the loop
background(10);
noStroke();
fill(255);
for (let i = 0; i < 40; i++) {
	const n = noise(i * 0.2 + cos(a) * R, 5 + sin(a) * R);   // returns to its start at progress 1
	circle(width * (i + 0.5) / 40, height * (0.2 + 0.6 * n), height * 0.02);
}
```

Non-integer multiples (`sin(a * 2.5)`) do not loop: the last frame jumps to the first.

## Trails and state

Unlike the C++ lane, p5 does not clear the canvas between frames. Two ways to do trails:

Stateless (preferred): redraw the past from the closed-form motion.

```js
background(0);
noStroke();
for (let j = 0; j < 12; j++) {
	const p = ctx.progress - j * 0.004;      // earlier moments; negative is fine, it is just sin/cos
	fill(255, 200 * (1 - j / 12));
	circle(width / 2 + cos(p * TWO_PI * 2) * width * 0.3, height / 2 + sin(p * TWO_PI * 2) * height * 0.3, height * 0.05);
}
```

Accumulating: skip `background()` or fade with a translucent rectangle. This depends on
every earlier frame having been drawn, so `make frame T=...` (which starts at that
frame) will not match the same moment in a full render. Judge such sketches with
`make sheet` or `make draft`.

```js
noStroke();
fill(0, 20);
rect(0, 0, width, height);                  // fades previous content
fill(255);
circle(width * ctx.progress, height / 2, height * 0.05);
```

## WEBGL

WEBGL works in headless Chromium (software or GPU rendering, whichever is available).
Origin is the canvas centre; lights and materials behave as in a browser.

```js sketch
function setup() { createCanvas(ctx.w, ctx.h, WEBGL); }
function draw() {
	background(20, 20, 40);
	lights();
	noStroke();
	fill(255, 120, 60);
	rotateY(ctx.progress * TWO_PI);
	rotateX(0.5);
	box(min(width, height) * 0.4);
}
```

## Colour

```js
colorMode(HSB, 255);                        // same 0..255 ranges as the C++ lane's fromHsb
background('#080a14');                      // hex strings are exact whatever colorMode is active
noStroke();
fill(map(ctx.progress, 0, 1, 0, 255), 200, 255);
rect(0, 0, width * 0.5, height);
colorMode(RGB, 255);                        // put it back if later code assumes RGB
```

## Parameters

`sketches/<name>/params.json` is passed as `ctx.params`. Always give a default.

```js
const n = ctx.params.count ?? 12;
background(0);
fill(255);
noStroke();
for (let i = 0; i < n; i++) circle((i + 0.5) * width / n, height / 2, height * 0.05);
```

## Assets

Any file in the sketch's own directory is served, and relative paths resolve against it.
Load in `preload()`.

```js sketch
let dot;
function preload() { dot = loadImage('dot.png'); }
function setup() { createCanvas(ctx.w, ctx.h); }
function draw() {
	background(30);
	image(dot, width * ctx.progress, height / 2, 32, 32);
}
```

(The test above writes a `dot.png` next to the sketch before running it.) A missing file
fails the render immediately with the file name.

## Errors you will see

The driver prints the first useful lines and exits non-zero (`make check` shows them):

```text
sketch error: draw() threw at frame 0: ReferenceError: nope is not defined
    at draw (sketch.js:4:3)
sketch error: Error: boom in setup
    at setup (sketch.js:2:9)
sketch error: sketch.js must define function draw()
sketch error: missing file: /missing.png (404). Assets load relative to the sketch's directory
sketch error: setup() did not finish within 15000 ms (infinite loop, or a missing/slow asset?)
sketch error: draw() did not return within 60000 ms at frame 120 (infinite loop?)
web engine not installed: run make setup-web
```

A syntax error is caught first by `node --check` in `make check`, which prints the file,
line and caret.

## Performance

Frame cost is drawing plus a PNG encode of the canvas. Thousands of shapes per frame
are fine; tens of thousands get slow at 1080p. `make sheet` and `make draft` render the
full clip, so keep an eye on them: `make smoke` (tiny resolution) is the quick sanity
check, `make frame T=...` the quick visual one (for stateless sketches).
