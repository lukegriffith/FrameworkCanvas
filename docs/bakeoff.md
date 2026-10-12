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

Run on 2026-10-12. Each cell is one fresh agent (same model for all ten). Sketches are kept
in `sketches/_bakeoff/<brief>-<lane>/`; render one with e.g.
`make sheet SKETCH=_bakeoff/B3-web FORMAT=instagram-portrait VIDEO_DURATION=10`.

| Brief | Lane | Raw first check | Real first check | Checks / sheets | Time (s) | Agent tokens | Quality | Seamless loop |
|---|---|---|---|---|---|---|---|---|
| B1 vortex | web | fail* | pass | 2 / 2 | 60 | 36.6k | 4 | yes (1.01) |
| B1 vortex | C++ | fail* | pass | 4 / 3 | 95 | 44.8k | 4 | yes (1.00) |
| B2 type | web | fail* | pass | 2 / 2 | 45 | 38.1k | 4 | yes (0.82) |
| B2 type | C++ | pass | pass | 2 / 2 | 66 | 43.8k | 3 | yes (1.11) |
| B3 ridgelines | web | fail* | pass | 5 / 5 | 104 | 48.5k | 5 | yes (1.11) |
| B3 ridgelines | C++ | pass | pass | 1 / 1 | 58 | 35.2k | 3 | yes (1.04) |
| B4 glow | web | pass | pass | 1 / 1 | 68 | 35.6k | 4 | yes (0.92) |
| B4 glow | C++ | pass | pass | 1 / 1 | 53 | 36.9k | 3 | yes (0.98) |
| B5 grid | web | fail* | pass | 2 / 2 | 37 | 33.1k | 5 | yes (1.10) |
| B5 grid | C++ | pass | pass | 1 / 1 | 41 | 32.4k | 5 | yes (1.13) |

\* Not a sketch failure. The agent put all its make options into one shell variable, zsh did not
split it, and make reported a confusing "both sketch.cpp and sketch.js" error. Five of the
first six agents hit this. It was fixed afterwards in the Makefile (`SKETCH` must now be a
single word, with a clear message), and the later C++ prompts warned against it, which is why
"raw first check" differs between the lanes. "Real first check" is the first check that actually
ran the sketch, taken from the agent's own report.

Seamless loop is measured, not claimed: the mean pixel difference between the last and first
frame divided by the difference between two consecutive frames. A seamless loop gives about
1.0, shown in brackets. All ten videos were also verified with ffprobe for the right size and
frame count.

### Summary

| | Web (p5.js) | C++ (openFrameworks) |
|---|---|---|
| Finished with correct output | 5 / 5 | 5 / 5 |
| Real first-try success | 5 / 5 | 5 / 5 |
| Mean time | 63 s | 63 s |
| Mean agent tokens | 38.4k | 38.6k |
| Mean quality (1 to 5) | 4.4 | 3.6 |
| Seamless loops | 5 / 5 | 5 / 5 |

### Conclusion

1. **Reliability and effort were indistinguishable.** Both lanes finished every brief, with
   the same success rate, the same mean time and the same token cost. The tooling (check,
   sheet, smoke) works equally well in both.
2. **Quality favoured the web lane, 4.4 to 3.6.** The gap came from three briefs. The C++
   ridgelines had spiky back layers and a half-hidden sun. The C++ type used a thin regular
   font because the default system font offers no weight choice. The C++ glow field clumped
   into one corner for a stretch. On the grid brief both scored 5. Web sketches also got
   gradients, soft shadows and alpha layering with less effort.
3. **Keep web as the default for 2D work** and C++ for custom shaders and heavy compute, as
   `CLAUDE.md` already recommends. The data does not justify going further than that.

### Caveats

- One run per cell, one model, five briefs. Treat the quality gap as a signal, not a result.
- The reviewer who scored quality also built the harness, and scored from contact sheets
  only. Scores are subjective.
- The web runs overlapped in time and the C++ runs did not, so timings are rough. Prompts
  for the later C++ runs added a warning about shell quoting.
- No brief required a custom GLSL shader, which is the C++ lane's stated strength, so that
  claim is untested here.
- Agents did not play the videos. They judged from contact sheets, as an agent would in
  normal use.
