# Bake-off: web lane (p5.js) vs C++ lane (openFrameworks)

Question: for an agent building a looping social-media asset from a one-paragraph brief,
which lane gets to a good result with fewer failures and less effort? The answer decides
the default recommendation in CLAUDE.md.

## Method

- 5 briefs, each built once per lane by a **fresh agent** that is told only the brief, the
  lane, and to read `CLAUDE.md` and follow its workflow. 10 runs total.
- Agents work in their own sketch directory (`sketches/bake-<id>-web`, `sketches/bake-<id>-cpp`),
  touch nothing else and commit nothing.
- C++ runs execute one at a time (they share one build directory); web runs may overlap.
- Each agent reports the numbers below from its own log. The reviewer then looks at the
  final contact sheet and assigns the quality score.

## Briefs

| ID | Brief | Format | Duration |
|---|---|---|---|
| B1 | **Particle vortex.** Several hundred glowing particles spiral around a dark centre in a teal and orange palette, with soft trails. Must loop seamlessly. | `square` 1080x1080 | 6 s |
| B2 | **Type loop.** The word "SIGNAL" as kinetic typography on a dark background: letters rise, shift in colour and settle in a repeating wave. Large, centred, readable. Must loop seamlessly. | `instagram-story` 1080x1920 | 8 s |
| B3 | **Ridgeline landscape.** 8 to 10 layered mountain ridgelines in a dusk gradient (warm at the back, cool at the front) drifting slowly sideways with parallax. Must loop seamlessly. | `instagram-portrait` 1080x1350 | 10 s |
| B4 | **Glow field.** A smooth, bright, plasma-like field of 5 to 7 metaball blobs that drift, merge and separate, with a soft neon colour ramp. Must loop seamlessly. | `hd` 1920x1080 | 8 s |
| B5 | **Data grid.** A 14 by 14 grid of dots or bars whose size and colour ripple outward from the centre as a travelling wave, on a clean light background with two accent colours. Must loop seamlessly. | `square` 1080x1080 | 6 s |

"Seamless loop": the last frame leads cleanly back to the first. Final render command for
every run: `make video SKETCH=<sketch> FORMAT=<format> VIDEO_DURATION=<duration>`.

## What each agent reports

```text
FIRST_TRY_CHECK: pass | fail      (result of the first `make check` after writing the sketch)
CHECK_RUNS: <n>                   (total `make check` invocations)
SHEET_RUNS: <n>                   (total `make sheet` invocations)
SMOKE: pass | fail
FINAL_VIDEO: <path>
SELF_ASSESSMENT: <two sentences: does it match the brief, does it loop>
STRUGGLES: <what cost the most time or caused errors, one to three lines>
```

Wall time is taken from the agent run's own timing.

## Scoring

| Measure | How |
|---|---|
| First-try check | From `FIRST_TRY_CHECK`. Pass means the first write compiled/ran with no changes. |
| Iterations | `CHECK_RUNS + SHEET_RUNS`. Fewer is better. |
| Finished | Final video exists, correct size and frame count (`ffprobe`). |
| Wall time | Minutes from agent start to finish. |
| Quality 1 to 5 | Reviewer, from the final contact sheet only: 1 broken or blank, 2 recognisable but wrong, 3 meets brief with visible flaws, 4 good, 5 would post it. |
| Loop | Reviewer: first and last sheet cells consistent, no jump. yes / no. |

## Results

(Filled in after the runs; see below.)
