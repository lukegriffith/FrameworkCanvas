# Web Engine Plan (Phase 5)

Goal: a second render lane where a sketch is one JavaScript file (p5.js) rendered
headless by Chromium via puppeteer, feeding the SAME agent loop and Makefile targets
as the C++ lane: `check`, `smoke`, `frame`, `sheet`, `draft`, `video`, `gif`, same
`out/<sketch>/` layout, same `Ctx` semantics. Then a bake-off decides which lane is
the default for new work.

Why: p5.js dwarfs openFrameworks in LLM training data, needs no compile step, and
runs headless on Linux/CI, which the OF lane cannot. The OF lane stays for
shader-heavy/native work.

Everything here follows the conventions of `docs/LLM_READINESS_PLAN.md`: one task
per commit, conventional commits, acceptance criteria an agent can verify itself.

---

## Technical breakdown

### Lane selection

A sketch directory declares its engine by file name. No new flag to learn:

```
sketches/<name>/sketch.cpp   -> OF lane (existing)
sketches/<name>/sketch.js    -> web lane
both present                 -> make fails with a clear error
```

The Makefile detects this once:

```make
ifneq ($(wildcard sketches/$(SKETCH)/sketch.js),)
  ENGINE := web
  FRAME_EXT := png
else
  ENGINE := of
  FRAME_EXT := tga
endif
```

and every ffmpeg input pattern becomes `frame_%06d.$(FRAME_EXT)`. The media targets
(`frame`, `sheet`, `draft`, `smoke`, `video`, `gif`) keep their interfaces and ffmpeg
recipes; only the "produce frames" step dispatches on `$(ENGINE)`.

### Layout

```
engines/web/
├── package.json        # puppeteer + p5, pinned exact versions; private: true
├── package-lock.json   # committed: renders must be reproducible
├── shell.html          # minimal page: loads p5 from node_modules, then harness.js
├── harness.js          # browser side: ctx, seeding, frame stepping (never edited by agents)
└── render.mjs          # node side: puppeteer driver, CLI (never edited by agents)
```

`node_modules` stays inside `engines/web/` and is git-ignored. No CDN: renders must
work offline and never change because a CDN bumped a version.

### The contract, JS side

Same `Ctx` fields as `src/Sketch.h`, exposed as a frozen global `ctx`:

```js
// Available to the sketch at all times:
// ctx.w, ctx.h        canvas size in pixels
// ctx.t               seconds; ALWAYS frame / fps (deterministic)
// ctx.progress        t / duration, wrapped to [0,1)
// ctx.frame           0-based frame index
// ctx.fps, ctx.duration
// ctx.seed            uint32
// ctx.recording       always true in this lane
// ctx.params          sketches/<name>/params.json or {}
```

A sketch is idiomatic global-mode p5, because that is what models know cold:

```js
function setup() {
  createCanvas(ctx.w, ctx.h);
}
function draw() {
  background(10);
  circle(width / 2, height / 2, min(width, height) * (0.3 + 0.05 * sin(ctx.progress * TWO_PI)));
}
```

Rules carried over verbatim from the OF lane: animate from `ctx.t`/`ctx.progress`
(never `frameCount`, `millis()`, or wall clock), integer multiples of
`ctx.progress * TWO_PI` for seamless loops, size from `ctx.w`/`ctx.h`.

### Determinism (harness.js responsibilities)

1. Before p5 loads: replace `Math.random` with a seeded mulberry32 PRNG derived from
   `ctx.seed`. Models reach for `Math.random()` constantly; it must be reproducible.
2. Via p5's init hook: call `randomSeed(ctx.seed)` and `noiseSeed(ctx.seed)` before
   the user's `setup()` runs, so setup-time randomness is reproducible too.
3. After setup: `noLoop()`. p5's own animation loop never runs.
4. Per frame: the driver sets `ctx.frame`/`ctx.t`/`ctx.progress`, calls `redraw()`
   (synchronous in p5), then captures. Render speed can never affect output.
5. Freeze `ctx` between updates so a sketch cannot accidentally mutate it.

### Frame capture

`canvas.toDataURL('image/png')` evaluated in the page, base64-decoded in node,
written to `$(FRAMES_DIR)/frame_%06d.png`. Exact canvas pixels, no compositor, no
device-pixel-ratio surprises (driver also forces `pixelDensity(1)` and
`deviceScaleFactor: 1`). WEBGL sketches need `preserveDrawingBuffer`; harness sets
`setAttributes('preserveDrawingBuffer', true)` when the sketch requests WEBGL.
PNG encode is the per-frame cost (~30-60 ms at 1080p); acceptable, and drafts are
480p. If it ever matters, CDP screencast is the upgrade path; not in scope.

### Error surfacing (the web lane's "compiler")

There is no compile step, so runtime errors must be loud and short:

- `render.mjs` subscribes to `pageerror` and `console` events; any `pageerror` or
  console.error aborts with exit 1 and prints the message + stack, filtered to
  lines mentioning `sketch.js`.
- A sketch that defines no `draw` function, or whose `setup` throws, must produce a
  one-line diagnosis, not a hang: the driver wraps setup/first-frame in a 15 s
  timeout.
- `make check` for the web lane = `node --check sketch.js` (syntax) + render frame 0
  and report `OK` or the error. Slightly slower than the OF `check`, but it is the
  real thing: a frame either rendered or you see why not.

### render.mjs CLI

```
node engines/web/render.mjs \
  --sketch sketches/<name>/sketch.js \
  --frames-dir out/<name>/frames \
  --w 1920 --h 1080 --fps 30 --duration 10 \
  --start 0 --end 299 --seed 1 [--params sketches/<name>/params.json]
```

Mirrors the env-var interface of the OF binary one-to-one so the Makefile dispatch
is mechanical. One browser launch per invocation; frames stepped in a single page
(launch cost ~1 s, amortised). `--start/--end` skip capture but still step earlier
frames' ctx? No: frames are pure functions of ctx in this lane (noLoop, no
accumulation between redraws unless the sketch keeps its own state), so the driver
jumps straight to `--start`. Sketches that accumulate state get the same caveat as
the OF lane: judge them with `sheet`/`draft`, not `frame`.

### What stays shared

ffmpeg assembly (video/gif/sheet/drawtext labels), `FORMAT` table, `out/<sketch>/`
layout, `params.json` discovery, `SEED`/`RENDER_START`/`RENDER_END` variables,
`clean-*` targets, the skills' outer workflow. An agent switching lanes changes the
file extension and the drawing API, nothing else.

---

## Task list

Sizes: S = under 30 min, M = 30-90 min, L = half day.

### Phase W0: renderer core

- [ ] **W0.1 (S)** `engines/web/package.json` (`private: true`, exact-pinned
  `puppeteer` and `p5`) + `make setup-web` target that runs `npm ci` (or
  `npm install` when no lock yet) inside `engines/web/` and prints the installed
  Chromium path. Git-ignore `engines/web/node_modules/`. Accept: `make setup-web`
  succeeds from clean checkout; second run is a no-op.

- [ ] **W0.2 (M)** `shell.html` + `harness.js`: ctx global, mulberry32
  `Math.random` override, p5 init hook calling `randomSeed`/`noiseSeed`,
  `pixelDensity(1)`, `noLoop()` after setup, `window.__step(frame)` that updates
  ctx and calls `redraw()`, `window.__capture()` returning the canvas dataURL.
  Accept: opening shell.html with a trivial sketch in headful Chrome renders; two
  `__step(5)` + `__capture()` calls return byte-identical dataURLs.

- [ ] **W0.3 (L)** `render.mjs`: CLI above, puppeteer launch, error/console
  forwarding with sketch.js-filtered stacks, 15 s setup timeout, frame loop
  writing PNGs, `--params` loading, exit codes (0 ok, 1 sketch error, 2 bad args).
  Accept: 45 PNGs from `--w 480 --h 270 --fps 15 --duration 3`; a sketch with a
  thrown error in draw exits 1 printing the line number; rerunning with same seed
  produces byte-identical frame 0.

- [ ] **W0.4 (S)** Determinism torture test (plain node script under
  `engines/web/test/`): sketch using `random()`, `noise()`, and `Math.random()`;
  render frames 10-12 twice with `--seed 7`, assert identical bytes; render with
  `--seed 8`, assert different. Accept: script exits 0 and is wired as
  `make test-web`.

### Phase W1: Makefile integration

- [ ] **W1.1 (M)** Engine detection block (`ENGINE`, `FRAME_EXT`), error when both
  sketch.cpp and sketch.js exist, error when sketch.js exists but
  `engines/web/node_modules` is missing ("run make setup-web"). Parametrise every
  ffmpeg input pattern on `$(FRAME_EXT)`. Accept: OF-lane `make video` and
  `make sheet` for the demo sketch still work unchanged (regression check).

- [ ] **W1.2 (M)** Dispatch the frame-producing step: a `render-frames` internal
  target that runs the OF binary (ENGINE=of) or `render.mjs` (ENGINE=web) with
  identical variable plumbing; `video`, `gif`, `sheet`, `frame`, `smoke`, `draft`
  all use it; web lane skips the `build` prerequisite. Accept: with a trivial
  `sketches/web-demo/sketch.js`, all six targets produce their documented outputs
  in `out/web-demo/`.

- [ ] **W1.3 (S)** Web `make check`: `node --check` + render frame 0, prints `OK`
  or the error, under ~5 s. Accept: deliberate typo -> file:line in under 15
  lines; fixed -> `OK`.

### Phase W2: template, examples, parity

- [ ] **W2.1 (S)** `sketches/_template-web/sketch.js`: commented skeleton mirroring
  `_template`'s rules. Accept: `cp -r`, `make check SKETCH=foo`, `make smoke
  SKETCH=foo` all pass.

- [ ] **W2.2 (M)** Port two examples to JS for side-by-side reference:
  `_examples/web-particles` (closed-form seeded motion) and `_examples/web-noise`
  (looping noise via cos/sin trick). Accept: `make sheet` on each shows a looping
  animation; `make smoke` passes; first/last sheet cells look like neighbours.

- [ ] **W2.3 (S)** Parity harness check: render the same minimal "progress bar +
  rotating square" sketch in both lanes at 480x270, assert both lanes' frame 30
  show the square at the same angle (visual check via one side-by-side montage,
  not pixel equality). Accept: montage committed to nothing, just verified and
  described in the commit message.

### Phase W3: docs and skills

- [ ] **W3.1 (M)** CLAUDE.md: "two lanes" section (how selection works, when to
  pick which: web = default for 2D/canvas work, OF = shaders/native/perf), the JS
  ctx contract, determinism notes (`Math.random` is seeded; never use
  `frameCount`/`millis`), link to `docs/P5_NOTES.md`. Accept: agent given only
  CLAUDE.md can produce a web sketch and a sheet without reading harness code.

- [ ] **W3.2 (M)** `docs/P5_NOTES.md`: short p5-in-this-harness notes, NOT a p5
  tutorial (models know p5): what the harness already did (seed, noLoop,
  pixelDensity), WEBGL + preserveDrawingBuffer, coordinate/angle modes, common
  harness-specific errors verbatim from render.mjs. Every snippet verified by
  rendering. Accept: snippets run; errors quoted match reality.

- [ ] **W3.3 (S)** Update `new-sketch` and `render` skills for lane choice; update
  README (two-lane quick start, requirements gain Node 20+). Accept: skills list
  both lanes; README quick start works as written.

### Phase W4: bake-off (decides the default)

- [ ] **W4.1 (S)** `docs/bakeoff.md`: 5 briefs spanning the range (looping particle
  vortex 1080x1080; typographic loop; noise landscape portrait; shader-ish glow
  effect; data-ish grid animation), scoring sheet: first-try compile/render,
  iterations to acceptable sheet, wall time, subjective quality 1-5.

- [ ] **W4.2 (L)** Run each brief in both lanes with a fresh agent per run (10
  runs), fill the table, write a 10-line conclusion recommending the default lane
  and updating CLAUDE.md's guidance. Accept: table complete; CLAUDE.md updated to
  match the data.

---

## Order and risk

```
W0.1 -> W0.2 -> W0.3 -> W0.4
W1.1 -> W1.2 -> W1.3
W2.1 -> W2.2 -> W2.3
W3.1 -> W3.2 -> W3.3
W4.1 -> W4.2
```

Risky bits, front-loaded on purpose: W0.2/W0.3 (p5 lifecycle hooks and capture
determinism) and W1.2 (Makefile dispatch without breaking the OF lane). W1.1's
regression check is mandatory before anything merges.

Non-goals: screencast capture, node-canvas, three.js lane, bundlers/TypeScript,
hot reload, browser-based preview UI. Keep the surface identical to the OF lane.
