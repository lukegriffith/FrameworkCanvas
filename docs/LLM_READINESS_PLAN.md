# LLM Readiness Plan

Goal: make FrameworkCanvas a framework an LLM agent can one-shot assets from.
Agent writes one sketch file, runs a short make loop, sees images of its own
output, fixes, ships MP4/GIF. Harness code is never touched by the agent.

Each task below is self-contained, has acceptance criteria, and lists its
dependencies. Do them in order within a phase. Commit per task using
conventional commits (`feat:`, `fix:`, `docs:`, `refactor:`, `chore:`).

Verify every build-touching task with `make Release` and `make draft` (once
T2.4 exists) before committing.

---

## Technical breakdown

### Current flow

```
Makefile ─env vars─> ofApp::setup() reads getenv
                     ofApp::draw() renders demo into recordFbo, saves TGA per frame
                     ofApp::update() calls ofExit() after totalFrames
Makefile ─ffmpeg──> frames/*.tga -> output.mp4 / output.gif
```

Problems:
1. Demo drawing lives inside `ofApp::drawScene`, next to capture plumbing.
2. Time is `ofGetFrameNum()`. App mode 60fps, record mode 30fps: different speed.
3. Record mode throttled to realtime by `ofSetFrameRate(videoFPS)`.
4. `ofSetWindowShape(videoWidth, videoHeight)` resizes window to output size.
5. No way to render one frame or a subset. No image an agent can look at.
6. OF make output is thousands of lines; errors buried.
7. One sketch per repo.

### Target flow

```
sketches/<name>/sketch.cpp   implements Sketch, exports makeSketch()
src/harness/*                ofApp, Recorder, Settings  (agent never edits)
src/Sketch.h                 Ctx struct + Sketch interface  (the contract)

make check  SKETCH=name            compile only, filtered errors
make frame  SKETCH=name T=2.5      one PNG at t=2.5s        -> out/name/frame_2.50.png
make sheet  SKETCH=name N=8        contact sheet of 8 frames -> out/name/sheet.png
make draft  SKETCH=name            480p 3s 15fps mp4         -> out/name/draft.mp4
make video  SKETCH=name FORMAT=instagram-story               -> out/name/name_instagram-story.mp4
make gif    SKETCH=name FORMAT=gif-web                       -> out/name/name_gif-web.gif
make smoke  SKETCH=name            3 frames, fail if uniform colour
```

### How sketch selection works in the OF makefile

`compile.project.mk` discovers sources with `find $(PROJECT_ROOT)/src` and
additionally compiles anything listed in `PROJECT_EXTERNAL_SOURCE_PATHS`
(checked in `config.project.mk` lines ~297-321). So:

```make
SKETCH ?= demo
override PROJECT_EXTERNAL_SOURCE_PATHS := $(abspath sketches/$(SKETCH))
```

must be set **before** `include .../compile.project.mk`. Only the chosen
sketch directory is compiled and linked. Object files are keyed by path under
`obj/`, so switching `SKETCH` recompiles only the sketch file, not the harness.

### The contract (`src/Sketch.h`)

```cpp
#pragma once
#include "ofMain.h"
#include <memory>

struct Ctx {
    int   w = 0, h = 0;        // render size in pixels
    float t = 0;               // seconds since start. Record mode: frame / fps (deterministic)
    float progress = 0;        // t / duration, wrapped to [0,1). Use for seamless loops
    int   frame = 0;           // frame index, 0-based
    float fps = 30;
    float duration = 10;       // seconds
    uint32_t seed = 1;         // ofSeedRandom(seed) is called before setup()
    bool  recording = false;
    ofJson params;             // contents of sketches/<name>/params.json, or empty object
};

class Sketch {
public:
    virtual ~Sketch() = default;
    virtual void setup(const Ctx&) {}
    virtual void draw(const Ctx&) = 0;   // called inside an FBO sized w x h, origin top-left
    virtual void keyPressed(int key, const Ctx&) {}
    virtual void mousePressed(int x, int y, int button, const Ctx&) {}
};

// Each sketch translation unit defines exactly one of these.
std::unique_ptr<Sketch> makeSketch();
```

Rules the harness guarantees:
- `draw` is always called with the FBO bound and cleared to black. Sketch may
  call `ofBackground` / `ofClear` itself to override.
- Both app mode and record mode call `draw` with identical `Ctx` semantics.
  In app mode `t` is wall clock since setup. In record mode `t = frame / fps`.
- `progress` wraps, so `sin(progress * TWO_PI)` loops perfectly.

### Environment variables the harness reads

Existing: `OUTPUT_TYPE`, `VIDEO_WIDTH`, `VIDEO_HEIGHT`, `VIDEO_FPS`, `VIDEO_DURATION`.

New:
- `RENDER_START`, `RENDER_END`: inclusive frame range to render in record mode.
  Default `0` and `totalFrames-1`. Frames outside the range are skipped
  (harness still advances `t` correctly: it just does not call `draw` or save).
- `SEED`: uint32, default 1.
- `PARAMS_FILE`: absolute path to a JSON file. Resolve with `std::ifstream`,
  **not** `ofFile` (see CLAUDE.md path gotcha).
- `FRAMES_DIR`: absolute path for TGA output. Default `bin/data/frames`.
  Lets each sketch write to `out/<name>/frames/` and avoids cross-sketch
  collisions.

### Record-mode speed

Replace `ofSetFrameRate(videoFPS)` in record mode with:

```cpp
ofSetVerticalSync(false);
ofSetFrameRate(0);          // 0 = uncapped in OF
```

Keep a small preview window (e.g. 480 wide, aspect-matched), never
`ofSetWindowShape(videoWidth, videoHeight)`. The FBO is the only thing that
must be output-sized.

### Contact sheet with ffmpeg

Given `N` frames spread evenly over `TOTAL` frames:

```sh
STEP=$(( TOTAL / N ))
ffmpeg -y -i frames/frame_%06d.tga \
  -vf "select='not(mod(n,$STEP))',scale=480:-1,tile=4x2" \
  -frames:v 1 out/<name>/sheet.png
```

For `N=8` use `tile=4x2`; general case `tile=$(COLS)x$(ROWS)` with
`COLS=4`, `ROWS=ceil(N/4)`. Cheaper alternative: render only the N needed
frames via repeated `RENDER_START=RENDER_END=k` runs. Start with the simple
full-render version; optimise later if needed.

### Filtering build output

```make
check:
	@bash -o pipefail -c '$(MAKE) Release 2>&1 | grep -E --line-buffered -A3 "error:|Error [0-9]|undefined|ld: " || true; exit $${PIPESTATUS[0]}'
```

Print a single `OK` line on success so the agent gets a clear signal.

---

## Task list

Sizes: S = under 30 min, M = 30-90 min, L = half day.

### Phase 0: quick fixes (no new structure)

- [ ] **T0.1 (S)** Uncap record mode. In `ofApp::setup` video branch, replace
  `ofSetFrameRate(videoFPS)` with `ofSetVerticalSync(false); ofSetFrameRate(0);`.
  Remove `ofSetWindowShape(videoWidth, videoHeight)`; keep default window.
  Accept: `make video VIDEO_DURATION=10` finishes in well under 10 s wall
  time at 1080p. Output MP4 still 300 frames (`ffprobe -count_frames`).

- [ ] **T0.2 (S)** README says PNG in three places; change to TGA. Remove the
  stale "Custom Animation" code snippet that calls `saveFrame()` from user
  code. Accept: `grep -n PNG README.md` returns nothing.

- [ ] **T0.3 (S)** Add `.gitignore` entries: `out/`, `*.mp4`, `*.gif`
  (keep `!small.gif` for README). Accept: `git status` clean after a render.

### Phase 1: sketch / harness split

- [ ] **T1.1 (S)** Create `src/Sketch.h` with the `Ctx` struct and `Sketch`
  interface exactly as in the contract above. No other changes.
  Accept: `make Release` still builds.

- [ ] **T1.2 (M)** Move `src/ofApp.h`, `src/ofApp.cpp`, `src/main.cpp` into
  `src/harness/`. OF's `find src` is recursive so no makefile change needed.
  Accept: `make Release` builds, `make run` shows demo unchanged.

- [ ] **T1.3 (M)** Extract env parsing into `src/harness/Settings.h/.cpp`:
  `struct Settings { string outputType; int w, h, fps, duration; int renderStart, renderEnd; uint32_t seed; string paramsFile, framesDir; static Settings fromEnv(); }`.
  Implement new vars (`RENDER_START`, `RENDER_END`, `SEED`, `PARAMS_FILE`,
  `FRAMES_DIR`) with defaults from the breakdown. Load `PARAMS_FILE` with
  `std::ifstream` into `ofJson`; empty object on missing file.
  Accept: unit-free check: `SEED=7 make run` prints `seed=7` in the startup log.

- [ ] **T1.4 (M)** Create `sketches/demo/sketch.cpp`: move the rotating
  circles from `drawScene` into a `DemoSketch : Sketch` using `ctx.t` and
  `ctx.progress` instead of `ofGetFrameNum()`. Define `makeSketch()`.
  Accept: file compiles once T1.5 wires it in.

- [ ] **T1.5 (M)** Makefile: add `SKETCH ?= demo` and
  `override PROJECT_EXTERNAL_SOURCE_PATHS := $(abspath sketches/$(SKETCH))`
  before the `include compile.project.mk`. Fail early with a clear message if
  `sketches/$(SKETCH)/sketch.cpp` does not exist.
  Accept: `make Release` links `sketches/demo/sketch.cpp`; `make Release SKETCH=nope` prints `sketch 'nope' not found` and exits 1.

- [ ] **T1.6 (M)** Rewire `ofApp` to own `std::unique_ptr<Sketch>` and a
  `Ctx`. `setup`: build Settings, `ofSeedRandom(seed)`, allocate FBO,
  `sketch->setup(ctx)`. `draw`: fill `ctx` (t, progress, frame), bind FBO,
  clear black, `sketch->draw(ctx)`, unbind, preview-draw, then save if
  recording and `frame` in `[renderStart, renderEnd]`. Forward
  `keyPressed`/`mousePressed`. Delete `drawScene`.
  Time rule: record mode `t = frame / fps`; app mode `t = ofGetElapsedTimef()`.
  Accept: `make run` and `make video` both show the demo at the same speed
  (circle completes one rotation per `duration` seconds in both).

- [ ] **T1.7 (S)** Honour `FRAMES_DIR` in `saveFrame`, create dir if missing.
  Makefile: set `FRAMES_DIR := $(abspath out/$(SKETCH)/frames)` and use it in
  the ffmpeg inputs and `clean-frames`. Output files go to `out/$(SKETCH)/`.
  Accept: `make video` produces `out/demo/demo.mp4` and leaves no `bin/data/frames`.

- [ ] **T1.8 (S)** Honour `RENDER_START`/`RENDER_END`. Harness exits after
  `renderEnd` instead of `totalFrames`. Frames before `renderStart` are
  skipped without calling `draw` or saving (still increment `frame`).
  Accept: `RENDER_START=10 RENDER_END=12 make video` writes exactly 3 TGAs
  named `frame_000010.tga` .. `frame_000012.tga`.

### Phase 2: feedback loop targets

- [ ] **T2.1 (S)** `make check`. Compile only, filtered output as in the
  breakdown, prints `OK` on success. Accept: introduce a deliberate typo in
  the demo sketch; `make check` output is under 15 lines and includes the
  file:line. Fix typo; `make check` prints `OK`.

- [ ] **T2.2 (M)** `make frame T=<seconds>`. Computes `F = round(T * FPS)`,
  runs record mode with `RENDER_START=RENDER_END=F`, converts the single TGA
  to `out/$(SKETCH)/frame_$(T).png` via ffmpeg, cleans frames. Default `T=0`.
  Accept: `make frame T=2.5` produces a PNG; agent can `Read` it.

- [ ] **T2.3 (M)** `make sheet N=<count>`. Full record-mode run, then ffmpeg
  `select`+`tile` into `out/$(SKETCH)/sheet.png`, 4 columns, each cell 480px
  wide. Default `N=8`. Accept: `make sheet` produces a 4x2 grid PNG whose
  cells visibly differ across time for the demo sketch.

- [ ] **T2.4 (S)** `make draft`. Alias:
  `$(MAKE) video VIDEO_WIDTH=480 VIDEO_HEIGHT=270 VIDEO_FPS=15 VIDEO_DURATION=3 VIDEO_FILENAME=draft.mp4`.
  Respect `FORMAT` aspect if given (see T2.6). Accept: completes in a few
  seconds after compile.

- [ ] **T2.5 (M)** `make smoke`. Render frames 0, mid, last at 320x180. For
  each, use `ffmpeg -vf signalstats -f null -` or ImageMagick `identify -format "%[standard-deviation]"`
  to detect a uniform image; fail with `frame N is blank` if stddev below a
  small threshold. Accept: demo passes; a sketch whose `draw` is empty fails.

- [ ] **T2.6 (M)** Replace preset targets with a `FORMAT` table:

  ```make
  FORMATS := hd:1920x1080x30 4k:3840x2160x30 square:1080x1080x30 portrait:1080x1920x30 \
             instagram:1080x1080x30 instagram-story:1080x1920x30 instagram-portrait:1080x1350x30 \
             gif-small:400x400x15 gif-web:800x600x20 gif-square:500x500x24
  ```

  Parse `FORMAT=name` into `VIDEO_WIDTH/HEIGHT/FPS`. Keep the old target
  names as one-line aliases calling `$(MAKE) video FORMAT=...` so docs stay
  valid. Output filename `out/$(SKETCH)/$(SKETCH)_$(FORMAT).mp4`.
  Accept: `make video FORMAT=instagram-story` and `make instagram-story`
  produce the same file.

- [ ] **T2.7 (S)** `make help` rewritten around the agent loop:
  `check -> frame/sheet -> draft -> video/gif`. List `SKETCH`, `FORMAT`,
  `T`, `N`, `SEED`, `PARAMS_FILE`. Accept: under 40 lines.

### Phase 3: agent documentation and examples

- [ ] **T3.1 (M)** Rewrite `CLAUDE.md`. Sections: purpose; the loop (check,
  sheet, draft, final); the `Ctx` contract verbatim; "never edit
  `src/harness/`"; where outputs land; OF 0.12.1 cheat sheet (see T3.2);
  commit conventions. Keep the existing path-resolution gotcha.
  Accept: a fresh agent given only CLAUDE.md can produce a new sketch and
  a sheet without reading harness code.

- [ ] **T3.2 (M)** OF 0.12.1 cheat sheet, as `docs/OF_CHEATSHEET.md`, linked
  from CLAUDE.md. Cover: coordinate system and `ofTranslate`/`ofRotateDeg`
  (not `ofRotate`); `ofColor::fromHsb`, `ofSetColor` with alpha and
  `ofEnableAlphaBlending`; `ofEnableBlendMode(OF_BLENDMODE_ADD)`;
  `ofSetCircleResolution`; `ofNoise`/`ofSignedNoise`; `ofMesh` + `ofVboMesh`
  basics; `ofPolyline`/`ofPath`; `ofShader::load` with paths relative to
  `bin/data`; `ofTrueTypeFont::load`; `ofFbo` for feedback effects;
  `glm::vec2/vec3` instead of `ofVec2f`; common compile errors and fixes.
  Accept: every snippet compiles when pasted into a sketch.

- [ ] **T3.3 (M)** Example sketches under `sketches/_examples/` (each its own
  dir so `SKETCH=_examples/particles` works):
  `particles` (seeded, looping), `noise-field` (flow lines), `shader`
  (fullscreen frag from `bin/data/shaders/`), `type` (ofTrueTypeFont with a
  bundled OFL font). Each under 120 lines, heavily commented, uses
  `ctx.progress` for a seamless loop. Accept: `make sheet SKETCH=_examples/<x>`
  works for all four and `make smoke` passes.

- [ ] **T3.4 (S)** `sketches/_template/sketch.cpp`: minimal commented
  skeleton. Accept: copying it to `sketches/foo/` and running `make check SKETCH=foo` prints `OK`.

- [ ] **T3.5 (S)** `.claude/skills/new-sketch/SKILL.md`: scaffold from
  template, then run `check`, `sheet`, read the sheet, iterate, `draft`,
  final. `.claude/skills/render/SKILL.md`: how to pick FORMAT and verify
  output with `ffprobe`. Accept: skills listed by Claude Code.

### Phase 4: params without recompile (optional)

- [ ] **T4.1 (S)** Makefile: if `sketches/$(SKETCH)/params.json` exists,
  export `PARAMS_FILE` as its absolute path. Accept: demo reads
  `ctx.params.value("speed", 1.0f)` and `params.json` changes alter output
  without `make Release` recompiling anything.

- [ ] **T4.2 (S)** Document `params.json` pattern in CLAUDE.md with a
  3-key example. Accept: example sketch uses it.

### Phase 5: web engine spike (decide by data)

- [ ] **T5.1 (M)** `engines/web/index.html` + `sketch.js` (p5.js) with the
  same `Ctx` fields exposed as a global, plus `engines/web/render.mjs`
  (puppeteer): fixed timestep, `page.screenshot` per frame into
  `out/<name>/frames/frame_%06d.png`. Accept: `node render.mjs --w 480 --h 270 --fps 15 --duration 3` yields 45 PNGs.

- [ ] **T5.2 (S)** Makefile `ENGINE ?= of`. When `ENGINE=web`, `video`/`gif`/
  `sheet` skip the OF build and call `render.mjs`; ffmpeg steps unchanged
  (input pattern switches to `.png`). Accept: `make sheet ENGINE=web SKETCH=demo` works.

- [ ] **T5.3 (M)** Bake-off. Write 5 asset prompts in `docs/bakeoff.md`
  (e.g. "looping 1080x1080 particle vortex, teal/orange, 6 s"). Run each
  against both engines with a fresh agent, record first-try success,
  retries, wall time. Decide default engine from results. Accept: table filled.

---

## Order of execution

```
T0.1 T0.2 T0.3
T1.1 -> T1.2 -> T1.3 -> T1.4 -> T1.5 -> T1.6 -> T1.7 -> T1.8
T2.1 -> T2.2 -> T2.3 -> T2.4 -> T2.5 -> T2.6 -> T2.7
T3.1 -> T3.2 -> T3.3 -> T3.4 -> T3.5
T4.1 -> T4.2
T5.1 -> T5.2 -> T5.3
```

Phases 0 to 2 are the minimum viable change. Phase 3 is what makes agents
reliable. Phases 4 and 5 are independent and can be skipped or reordered.
