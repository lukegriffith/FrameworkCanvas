# CLAUDE.md

Guidance for Claude Code (claude.ai/code) working in this repository.

## What this is

FrameworkCanvas is a generator for social-media video and GIF art. You write ONE file
per asset, in one of two languages ("lanes"), and the harness handles timing, frame
capture and ffmpeg:

- `sketches/<name>/sketch.js`  : **web lane**, p5.js in headless Chromium
- `sketches/<name>/sketch.cpp` : **C++ lane**, openFrameworks

Never edit `src/harness/`, `src/Sketch.h` or `engines/` to make a sketch work; if the
harness seems wrong, say so instead.

## Which lane?

The file name decides: `sketch.js` runs in the web lane, `sketch.cpp` in the C++ lane
(having both is an error). Every `make` target below works identically in both lanes.

| Pick | When |
|---|---|
| Web (`sketch.js`) | Default for 2D canvas work: shapes, particles, type, generative patterns, noise fields, anything p5 does. No compile step, fastest to iterate, p5 is what you know best. |
| C++ (`sketch.cpp`) | Custom GLSL shaders (GL 2.1), heavy per-frame maths or very large particle counts, or work already written against openFrameworks. |

Evidence: in a 10-run comparison (`docs/bakeoff.md`) both lanes were equally reliable and
cost the same effort; web output scored higher on quality (4.4 vs 3.6 out of 5). It did not
test shader-heavy work.

First time using the web lane: `make setup-web` (installs Node packages and Chromium
under `engines/web/`; needs Node 20+). Then `cp -r sketches/_template-web sketches/<name>`.

## The loop (do this for every new asset)

```bash
cp -r sketches/_template-web sketches/<name>  # 1. scaffold (C++: sketches/_template). Or copy an example
# edit sketches/<name>/sketch.js (or sketch.cpp)
make check  SKETCH=<name>                     # 2. C++: compile. Web: syntax check + render frame 0. Prints OK or the error
make smoke  SKETCH=<name>                     # 3. fails if the output is blank
make sheet  SKETCH=<name>                     # 4. LOOK at out/<name>/sheet.png
make frame  SKETCH=<name> T=2.5               #    optional: one full-size still
make draft  SKETCH=<name>                     # 5. quick low-res MP4 check
make video  SKETCH=<name> FORMAT=instagram-story   # 6. final render
```

Step 4 is the important one. You cannot watch a video, but you can read an image:
open `out/<name>/sheet.png` with your image-reading tool. It is 8 labelled frames
spread over the whole clip. Judge composition, colour, motion between cells, loop
seam (first and last cell should look like neighbours), and anything blank or
off-screen. Iterate on `check` and `sheet` until it looks right, then render.

Where things land (all under `out/<name>/`, git-ignored):

| Command | Output |
|---|---|
| `make frame T=<sec>` | `frame_<sec>.png` |
| `make sheet [N=8]` | `sheet.png` |
| `make draft` | `draft.mp4` (480px wide, 15 fps, full duration) |
| `make video` | `<name>.mp4`, or `<name>_<format>.mp4` with `FORMAT=` |
| `make gif` | `<name>.gif`, or `<name>_<format>.gif` |

`SKETCH` defaults to `demo`. Nested names work: `SKETCH=_examples/particles`.
`make help` lists everything. The `overriding commands for target 'help'` warnings
printed by make are harmless noise from openFrameworks.

## The sketch contract (`src/Sketch.h`)

```cpp
struct Ctx {
    int   w, h;        // render size in pixels
    float t;           // seconds since start. Deterministic when recording (frame / fps)
    float progress;    // t / duration, wrapped to [0,1). Use for seamless loops
    int   frame;       // 0-based frame index
    float fps, duration;
    uint32_t seed;     // ofSeedRandom(seed) is already called before setup()
    bool  recording;
    ofJson params;     // sketches/<name>/params.json if present, else {}
};
class Sketch {
public:
    virtual void setup(const Ctx&) {}
    virtual void draw(const Ctx&) = 0;        // required
    virtual void keyPressed(int key, const Ctx&) {}
    virtual void mousePressed(int x, int y, int button, const Ctx&) {}
};
std::unique_ptr<Sketch> makeSketch();         // your file defines this
```

Guarantees: `draw` runs with a black, cleared, anti-aliased canvas of `ctx.w` x `ctx.h`,
origin top-left, current colour white. Interactive mode and video mode call `draw`
with identical semantics, so the animation looks the same in both.

### The same contract in the web lane

`ctx` is a read-only global with the same fields (`w h t progress frame fps duration seed
recording params`). A sketch is ordinary global-mode p5.js:

```js
function setup() { createCanvas(ctx.w, ctx.h); }   // size arguments are ignored: the canvas is always ctx.w x ctx.h
function draw() {
  background(10);
  circle(width / 2, height / 2, min(width, height) * (0.3 + 0.05 * sin(ctx.progress * TWO_PI)));
}
```

The harness seeds `random()`, `noise()` and `Math.random()` from `SEED`, fixes
`pixelDensity` at 1, makes `millis()` equal `ctx.t * 1000` and `frameCount` equal
`ctx.frame + 1`. `Date.now()` is not patched. Differences from the C++ lane: p5 does not
clear the canvas between frames (call `background()`), and `noise()` is 3D (loop it by
moving the sample point around a circle). Read `docs/P5_NOTES.md` before writing a
web sketch beyond the basics.

## Rules for good sketches

1. **Animate from `ctx.t` / `ctx.progress`.** Never `ofGetFrameNum()`, `ofGetElapsedTimef()`,
   `Date.now()`, `performance.now()` or the clock: they differ between app mode (60 fps
   window) and video mode (30 fps) and are not reproducible.
2. **Loop seamlessly.** `progress` wraps, so `sin(ctx.progress * TWO_PI * k)` with integer
   `k` returns to its start. Non-integer `k` does not loop. For noise, walk a circle
   in extra dimensions (see `_examples/noise-field`).
3. **Size from `ctx.w`/`ctx.h`.** The same sketch must work for 1080x1080, 1080x1920,
   1920x1080 and 4k. Use `float unit = std::min(ctx.w, ctx.h)`.
4. **Prefer stateless motion**: position as a closed-form function of `progress`
   (see `_examples/particles`). Anything that accumulates between frames (simulation,
   feedback FBOs) cannot be sampled with `make frame` because earlier frames are
   skipped. Check those with `make sheet` / `make draft` instead.
5. **Randomness is reproducible.** `ofRandom`/`ofNoise` are seeded from `SEED` (default 1).
   Generate random data in `setup()`, not `draw()`, so it does not flicker.
6. **Load assets in `setup()`** (web: `preload()`), never in `draw()`.
7. **Do not change `ctx.duration`-relative timing to make drafts faster.** `make draft`
   already keeps the full duration and lowers resolution and frame rate instead.

## Tunable parameters without recompiling

Put knobs in `sketches/<name>/params.json`. The Makefile finds it automatically (or pass
`PARAMS_FILE=path`) and the sketch reads it through `ctx.params`, so changing a value
only re-renders; it does not rebuild. This makes colour, speed and count tweaks cheap.

```json
{ "turns": 1, "pulses": 3, "hue": 0 }
```
```cpp
int pulses = ctx.params.value("pulses", 3);   // always pass a default: the file is optional
```

Keep anything that multiplies `ctx.progress * TWO_PI` an integer, or the loop will jump.
`sketches/demo` uses this.

## Reference material

- `docs/OF_CHEATSHEET.md` : (C++ lane) openFrameworks 0.12.1 API gotchas, compile-checked
  snippets, common errors. Read before writing non-trivial drawing code.
- `docs/P5_NOTES.md` : (web lane) what the harness changes about p5, looping recipes,
  trails, WEBGL, assets, every error message you can see. Every snippet is tested.
- `sketches/_template/` (C++) and `sketches/_template-web/` (web) : minimal commented skeletons.
- `sketches/_examples/web-particles`, `web-noise` : web ports of the particle and noise-field examples.
- `sketches/_examples/particles` : (C++) stateless seeded particles with trails, perfect loop.
- `sketches/_examples/noise-field` : looping noise flow lines, any aspect ratio.
- `sketches/_examples/shader` : fullscreen GLSL (`#version 120`) with embedded source.
- `sketches/_examples/type` : animated text using the system font.

## Options (any target)

| Variable | Meaning |
|---|---|
| `SKETCH=name` | which sketch directory under `sketches/` |
| `FORMAT=name` | sets size and fps: hd, 4k, square, portrait, instagram-feed, instagram-story, instagram-reel, instagram-portrait, gif-small, gif-web, gif-square, gif-instagram-feed, gif-instagram-story. Each is also a target: `make instagram-story` |
| `VIDEO_WIDTH` `VIDEO_HEIGHT` `VIDEO_FPS` `VIDEO_DURATION` | override anything; explicit values beat `FORMAT` |
| `SEED=n` | random seed |
| `RENDER_START` `RENDER_END` | inclusive frame range (partial render) |
| `T=sec`, `N=count` | for `frame` and `sheet` |

Defaults live in `config.make` (1920x1080, 30 fps, 10 s). `OF_ROOT` there must point at
an openFrameworks 0.12.1 install (`make setup` helps find it).

## Architecture (for harness changes only)

- `src/Sketch.h` : the contract above.
- `src/harness/main.cpp` : window creation (GL 2.1 context).
- `src/harness/ofApp.*` : owns the sketch, builds `Ctx` each frame, renders into a 4x MSAA
  FBO at output size, saves each frame, exits after the last.
- `src/harness/Settings.*` : reads env vars (`OUTPUT_TYPE`, `VIDEO_*`, `RENDER_*`, `SEED`,
  `PARAMS_FILE`, `FRAMES_DIR`) set by the Makefile.
- `Makefile` : `SKETCH` selection (compiles only `sketches/<name>/` via
  `PROJECT_EXTERNAL_SOURCE_PATHS`), the quiet `build` target, all media targets, the
  `FORMAT` table. Media targets depend on `build`, which prints nothing unless compilation fails.
- Record mode runs uncapped (no vsync, no frame-rate limit); time comes from the frame
  index, so rendering speed never changes the result.
- `engines/web/` : the web lane. `render.mjs` (puppeteer driver, CLI mirrors the C++ binary's
  env vars), `harness.js` / `harness-post.js` (browser side: `ctx`, seeding, forced canvas
  size, frame stepping), `test/run.mjs` (`make test-web`: determinism, contract, error
  handling, and every snippet in `docs/P5_NOTES.md`). p5 and puppeteer versions are pinned
  in `package.json`; the lock file is committed so renders are reproducible. Frames are PNG
  here (the C++ lane writes TGA); ffmpeg assembly is shared. The Makefile picks the lane
  via `ENGINE`/`FRAME_EXT` and calls the `render_frames` macro.

### Frame capture format
Frames are saved as uncompressed TGA, not PNG. PNG's zlib compression is the
dominant per-frame cost during recording (~9x slower at 1080p) and these are
throwaway intermediates that the Makefile deletes right after ffmpeg assembles the
final media, so there is no reason to pay for compression.

### Handling user-supplied file paths
`ofFile`/`ofToDataPath` resolve relative paths against `bin/data/`, not the
process CWD. Any path that comes from an env var or Makefile-forwarded CLI
arg (e.g. a script/config/asset path) will silently fail to open if passed
straight to `ofFile()`. Use `std::ifstream` directly, or resolve to an
absolute path before handing it to OF's file API. On the Makefile side,
apply `override VAR := $(abspath $(VAR))` for any such path variable;
plain `:=` won't override a value passed on the `make` command line.
(Inside a sketch, assets loaded with OF calls such as `ofImage::load` ARE relative
to `bin/data/`; that is intended.)

## Dependencies

- openFrameworks 0.12.1+, Xcode Command Line Tools, C++17 compiler
- FFmpeg (`brew install ffmpeg`), with `drawtext` for sheet time labels (default brew build has it)
- Web lane only: Node 20+ and `make setup-web`

## Commit message guidelines

Conventional commits: `feat:`, `fix:`, `docs:`, `refactor:`, `style:`, `test:`, `chore:`, `perf:`.
