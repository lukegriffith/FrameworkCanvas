---
name: render
description: Render a FrameworkCanvas sketch to its final MP4 or GIF for a target platform (Instagram, YouTube, web) and verify the file. Use when asked to export, render, produce or publish a sketch's video or GIF.
---

# Render

## Pick a FORMAT

| Destination | Command |
|---|---|
| YouTube / general 16:9 | `make video SKETCH=<n> FORMAT=hd` (4k: `FORMAT=4k`) |
| Instagram feed | `FORMAT=instagram-feed` (1080x1080) |
| Instagram portrait post | `FORMAT=instagram-portrait` (1080x1350) |
| Reels / Stories / TikTok / Shorts | `FORMAT=instagram-story` (1080x1920) |
| Small web GIF | `make gif SKETCH=<n> FORMAT=gif-web` (also gif-small, gif-square) |
| Instagram GIF | `make gif SKETCH=<n> FORMAT=gif-instagram-feed` or `gif-instagram-story` |

Duration: `VIDEO_DURATION=<seconds>` (whole seconds). Because the loop is defined by
`ctx.progress`, changing duration stretches the animation, it does not cut it.
Platform limits to keep in mind: Reels and Shorts want 15 to 60 s, GIFs get huge fast
(prefer under 6 s and a small format).

## Render

```bash
make video SKETCH=<n> FORMAT=<f> [VIDEO_DURATION=<s>] [SEED=<n>]
```

Output: `out/<n>/<n>_<f>.mp4` (or `.gif`). Works for both lanes (web `sketch.js`,
C++ `sketch.cpp`). Frames are rendered uncapped, so a 10 s 1080p clip takes seconds to
a few tens of seconds, not real time. Web lane needs `make setup-web` once.

## Verify (always)

```bash
F=out/<n>/<n>_<f>.mp4
ffprobe -v error -select_streams v:0 -count_frames \
  -show_entries stream=width,height,r_frame_rate,nb_read_frames,duration -of default=nw=1 "$F"
ls -lh "$F"
```

Check: width/height match the format, `nb_read_frames` equals duration x fps, size is
sane (MP4 well under 50 MB for 10 s; GIF under about 15 MB). Then look at the result:

```bash
ffmpeg -y -v error -i "$F" -vf "select='eq(n,0)+eq(n,N/2)+eq(n,LAST)',scale=480:-1,tile=3x1" -frames:v 1 out/<n>/check.png
```
(replace `N/2` and `LAST` with real frame numbers) and Read `out/<n>/check.png`. Confirm
the last frame leads cleanly back to the first.

Report the output path, resolution, duration, file size, and anything you could not
verify.
