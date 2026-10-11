# CLAUDE.md

Guidance for Claude Code (claude.ai/code) working in this repository.

## What this is

FrameworkCanvas turns openFrameworks (C++) into a generator for social-media video
and GIF art. You write ONE file, `sketches/<name>/sketch.cpp`. The harness handles
windows, timing, frame capture and ffmpeg. Never edit `src/harness/` or `src/Sketch.h`
to make a sketch work; if the harness seems wrong, say so instead.

## The loop (do this for every new asset)

```bash
cp -r sketches/_template sketches/<name>      # 1. scaffold (or copy an example)
# edit sketches/<name>/sketch.cpp
make check  SKETCH=<name>                     # 2. compile. Prints OK or filtered errors
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

## Rules for good sketches

1. **Animate from `ctx.t` / `ctx.progress`.** Never `ofGetFrameNum()`, `ofGetElapsedTimef()`
   or the clock: they differ between app mode (60 fps window) and video mode (30 fps).
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
6. **Load assets in `setup()`**, never in `draw()`.
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

- `docs/OF_CHEATSHEET.md` : openFrameworks 0.12.1 API gotchas, compile-checked snippets,
  common errors. Read before writing non-trivial drawing code.
- `sketches/_template/` : minimal commented skeleton.
- `sketches/_examples/particles` : stateless seeded particles with trails, perfect loop.
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

## Commit message guidelines

Conventional commits: `feat:`, `fix:`, `docs:`, `refactor:`, `style:`, `test:`, `chore:`, `perf:`.
