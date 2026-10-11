# FrameworkCanvas

> A complete openFrameworks digital art template with dual-mode video recording

![Demo Animation](small.gif)

FrameworkCanvas combines the power of openFrameworks with a streamlined make-based workflow, perfect for digital artists who want both interactive creativity and high-quality video output.

## ✨ Features

- **Dual-Mode Operation**: Switch between interactive app and video/GIF recording modes
- **Zero-Config Media Generation**: Frame-by-frame TGA export with automatic MP4/GIF creation
- **Multiple Output Formats**: Generate high-quality MP4 videos or web-friendly animated GIFs
- **Artist-Friendly Workflow**: Simple `make art` commands with preset formats
- **Neovim + Make Integration**: No Xcode required - pure terminal workflow
- **Customizable Output**: Configure resolution, framerate, duration, and quality
- **Self-Contained**: Everything managed through a single, powerful Makefile

## 🚀 Quick Start

```bash
# Clone and setup
git clone <your-repo>
cd FrameworkCanvas

# Check/locate your openFrameworks install (run this once after cloning)
make setup

# Interactive mode - opens a window for real-time creativity
make run

# Look at your work before rendering
make sheet     # contact sheet of the whole animation -> out/demo/sheet.png
make frame T=2.5  # one full-size still

# Create a video - generates MP4 automatically (output lands in out/<sketch>/)
make video

# Create an animated GIF - perfect for web sharing
make gif

# Quick presets
make instagram # 1080x1080 Instagram feed post
make gif-web   # 800x600 optimized GIF
make 4k        # 3840x2160 for high quality
make square    # 1080x1080 for general social media
```

Your art lives in `sketches/<name>/sketch.cpp`. Start from `sketches/_template`, or copy
an example from `sketches/_examples/`. Select a sketch with `SKETCH=<name>`
(default `demo`). See [CLAUDE.md](CLAUDE.md) for the full workflow, which is written to
work equally well for people and for coding agents.

## 🎨 Usage

### Basic Commands

```bash
make           # Build interactive .app
make run       # Build and run interactive app  
make video     # Create MP4 video (default settings)
make gif       # Create animated GIF (smaller, web-friendly)
make art       # Alias for 'make video'
make help      # Show complete usage guide
```

### Media Presets

**Instagram Optimized:**
```bash
make instagram         # 1080x1080 feed post (most common)
make instagram-story   # 1080x1920 stories & reels
make instagram-reel    # 1080x1920 reel format  
make instagram-portrait # 1080x1350 portrait post
```

**Video (MP4):**
```bash
make hd        # 1920x1080 @ 30fps
make 4k        # 3840x2160 @ 30fps  
make square    # 1080x1080 @ 30fps
make portrait  # 1080x1920 @ 30fps
```

**GIF (Optimized for Web):**
```bash
make gif-small           # 400x400 @ 15fps - tiny file size
make gif-web             # 800x600 @ 20fps - balanced quality/size
make gif-square          # 500x500 @ 24fps - social media ready
make gif-instagram-feed  # 1080x1080 @ 24fps - Instagram ready
make gif-instagram-story # 1080x1920 @ 24fps - Story format
```

### Custom Recording

```bash
# Override any video setting on the fly
make video VIDEO_WIDTH=2560 VIDEO_HEIGHT=1440 VIDEO_FPS=60 VIDEO_DURATION=15

# Custom GIF with specific settings
make gif VIDEO_WIDTH=600 VIDEO_HEIGHT=400 VIDEO_FPS=12 GIF_FILENAME=my_art.gif

# Custom filenames
make video VIDEO_FILENAME=my_artwork.mp4
make gif GIF_FILENAME=demo_animation.gif
```

## ⚙️ Configuration

Edit `config.make` to set your preferred defaults:

```make
# Output mode: "app" for interactive, "video" for recording
OUTPUT_TYPE = video

# Video settings
VIDEO_WIDTH = 1920
VIDEO_HEIGHT = 1080  
VIDEO_FPS = 30
VIDEO_DURATION = 10

# Optional. Default is <sketch>.mp4 / <sketch>.gif, written to out/<sketch>/
# VIDEO_FILENAME = my_video.mp4
# GIF_FILENAME = my_anim.gif

# FFmpeg path (if not in PATH)  
FFMPEG = ffmpeg
```

## 🛠️ Requirements

- **openFrameworks 0.12.1+** - [Download here](https://openframeworks.cc/download/)
- **Xcode Command Line Tools** - `xcode-select --install`
- **FFmpeg** - `brew install ffmpeg` (for video generation)
- **C++17 compatible compiler**

## 📁 Project Structure

```
FrameworkCanvas/
├── sketches/
│   ├── demo/                # default sketch (+ params.json)
│   ├── _template/           # copy this to start a new sketch
│   └── _examples/           # particles, noise-field, shader, type
├── src/
│   ├── Sketch.h             # the sketch contract (Ctx + Sketch)
│   └── harness/             # app, recorder, settings: not for sketch authors
├── docs/
│   ├── OF_CHEATSHEET.md     # openFrameworks 0.12.1 gotchas
│   └── LLM_READINESS_PLAN.md
├── out/                     # renders (git-ignored): out/<sketch>/...
├── config.make              # project configuration
├── Makefile                 # build + media workflow
└── CLAUDE.md                # workflow guide for agents and people
```

## 🎥 How Media Generation Works

### Video (MP4)
1. **Frame Generation**: App renders offscreen as fast as it can, saving each frame as uncompressed TGA
2. **H.264 Encoding**: FFmpeg converts frames to high-quality MP4 with H.264 codec
3. **Cleanup**: Temporary frames are automatically removed
4. **Output**: Final video saved to project directory

### GIF Animation  
1. **Frame Generation**: Same TGA frame sequence as video
2. **Palette Optimization**: FFmpeg generates optimal color palette for better quality
3. **GIF Encoding**: Two-pass process creates smooth, web-optimized animated GIF
4. **Cleanup**: Temporary frames and palette files removed automatically

The demo animation features rotating colorful circles - perfect for testing or as a starting point for your own creations.

## 🎯 Interactive Mode Features

- **Real-time Preview**: See your art as you create it
- **Manual Recording**: Press 'r' to start recording anytime
- **Live Stats**: FPS counter and frame information
- **Responsive Controls**: Full mouse and keyboard support

## 🧹 Cleanup Commands

```bash
make clean-frames    # Remove temporary frame files
make clean-video     # Remove generated MP4s
make clean-gif       # Remove generated GIF files
make clean-all       # Clean everything (build + frames + videos + gifs)
```

## 🔧 Advanced Usage

### Environment Variables

The app reads these environment variables (set by Makefile):

- `OUTPUT_TYPE` - "app" or "video"
- `VIDEO_WIDTH` - Frame width in pixels
- `VIDEO_HEIGHT` - Frame height in pixels  
- `VIDEO_FPS` - Frames per second
- `VIDEO_DURATION` - Recording duration in seconds
- `SEED`, `RENDER_START`, `RENDER_END`, `PARAMS_FILE`, `FRAMES_DIR` - set by the Makefile; see CLAUDE.md

### Writing a sketch

```bash
cp -r sketches/_template sketches/mything
make check  SKETCH=mything      # compile only, prints OK or the errors
make sheet  SKETCH=mything      # open out/mything/sheet.png
make video  SKETCH=mything FORMAT=instagram-story
```

A sketch is one class with a `draw(const Ctx&)` method. Animate from `ctx.t` (seconds) or
`ctx.progress` (0..1 over the clip, so `sin(ctx.progress * TWO_PI * k)` loops seamlessly),
and size things from `ctx.w` / `ctx.h`. Frame capture and timing are handled for you.

```cpp
#include "Sketch.h"

class MySketch : public Sketch {
public:
    void draw(const Ctx& ctx) override {
        ofSetColor(255, 100, 200);
        ofDrawCircle(ctx.w / 2.0f, ctx.h / 2.0f,
                     std::min(ctx.w, ctx.h) * (0.2f + 0.05f * std::sin(ctx.progress * TWO_PI)));
    }
};
std::unique_ptr<Sketch> makeSketch() { return std::make_unique<MySketch>(); }
```

Other useful make targets: `make smoke` (fail on blank output), `make draft` (fast low-res
MP4), `make help`. Per-sketch tunables go in `sketches/<name>/params.json`.

## 🤝 Contributing

This is a template project designed to be forked and customized for your own digital art creations. Feel free to:

- Add new preset formats
- Extend the animation system
- Integrate additional openFrameworks addons
- Improve the build system

## 📄 License

This project is open source and available under the MIT License.

## 🙏 Credits

Built with [openFrameworks](https://openframeworks.cc/) - A creative coding toolkit.

Video encoding powered by [FFmpeg](https://ffmpeg.org/).