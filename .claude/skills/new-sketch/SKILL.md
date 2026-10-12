---
name: new-sketch
description: Create a new FrameworkCanvas sketch (looping video/GIF art, p5.js or openFrameworks) from a brief, then iterate on it by looking at rendered contact sheets until it is right. Use when asked to make, generate or prototype a visual, animation, loop, or social-media asset in this repo.
---

# New sketch

Goal: turn a brief into `sketches/<name>/sketch.cpp` and check it by looking at images
of its output, not by guessing.

## 1. Pick a lane, then read first (once)

Default to the **web lane** (p5.js, `sketch.js`) for 2D work. Use the **C++ lane**
(openFrameworks, `sketch.cpp`) for custom GLSL shaders or very heavy per-frame work.
`CLAUDE.md` has the details. Both lanes use the same make commands below.

- `CLAUDE.md` for the `ctx` contract and rules (both lanes).
- Web: `docs/P5_NOTES.md`. C++: `docs/OF_CHEATSHEET.md`.
- The closest example in `sketches/_examples/`: `web-particles`, `web-noise` (web);
  `particles`, `noise-field`, `shader`, `type` (C++).
- Web lane not set up yet? Run `make setup-web` once (needs Node 20+).

## 2. Scaffold

Pick a short kebab-case name. Never reuse `demo`, `web-demo` or a name starting with `_`.

```bash
cp -r sketches/_template-web sketches/<name>     # web lane
cp -r sketches/_template sketches/<name>         # C++ lane
```

A directory has exactly one of `sketch.js` / `sketch.cpp`.

Decide before coding: duration (default 10 s), format (`FORMAT=`, default 1920x1080),
palette, and what loops. Put non-default values in `sketches/<name>/params.json` or pass
`VIDEO_DURATION=...` on every command.

## 3. Write, then loop until it looks right

```bash
make check SKETCH=<name>     # must print OK. Fix errors, rerun (web: syntax + a real frame 0 render).
make smoke SKETCH=<name>     # must pass. Fails on blank output.
make sheet SKETCH=<name> FORMAT=<final format>
```

Then READ `out/<name>/sheet.png` with the Read tool. Check against the brief:

- Is anything visible, centred, and sized sensibly for this format?
- Does it change from cell to cell? Is the motion what the brief asked for?
- Do the first and last cells look like neighbours (seamless loop)?
- Contrast and colour: is anything muddy, clipped, or unreadable?

Fix what is wrong and repeat. Use `make frame SKETCH=<name> T=<sec>` for a full-size look
at one moment (not valid for sketches that accumulate state between frames).
Stop iterating when the sheet matches the brief, not when it merely compiles.

## 4. Final check and render

```bash
make draft SKETCH=<name> FORMAT=<final format>    # fast MP4 sanity check
make video SKETCH=<name> FORMAT=<final format>    # or: make gif ...
```

Then follow the `render` skill to verify the file.

## Do not

- Edit `src/` or `engines/` to make a sketch work.
- Animate from `ofGetFrameNum()`, `ofGetElapsedTimef()`, `Date.now()`, `performance.now()`
  or any clock. Use `ctx.t` / `ctx.progress`.
- Declare it done after `check` alone. Always look at a sheet.
- Commit `out/` (git-ignored) or leave stray scratch sketches in `sketches/`.
