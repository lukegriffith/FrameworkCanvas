# FrameworkCanvas

> A complete openFrameworks digital art template with dual-mode video recording

FrameworkCanvas combines the power of openFrameworks with a streamlined make-based workflow, perfect for digital artists who want both interactive creativity and high-quality video output.

## ✨ Features

- **Dual-Mode Operation**: Switch between interactive app and video recording modes
- **Zero-Config Video Recording**: Frame-by-frame PNG export with automatic MP4 generation
- **Artist-Friendly Workflow**: Simple `make art` commands with preset formats
- **Neovim + Make Integration**: No Xcode required - pure terminal workflow
- **Customizable Output**: Configure resolution, framerate, duration, and quality
- **Self-Contained**: Everything managed through a single, powerful Makefile

## 🚀 Quick Start

```bash
# Clone and setup
git clone <your-repo>
cd FrameworkCanvas

# Interactive mode - opens a window for real-time creativity
make run

# Create a video - generates MP4 automatically
make art

# Quick presets
make square    # 1080x1080 for social media
make 4k        # 3840x2160 for high quality
make portrait  # 1080x1920 for vertical videos
```

## 🎨 Usage

### Basic Commands

```bash
make           # Build interactive .app
make run       # Build and run interactive app  
make video     # Create MP4 video (default settings)
make art       # Alias for 'make video'
make help      # Show complete usage guide
```

### Video Presets

```bash
make hd        # 1920x1080 @ 30fps
make 4k        # 3840x2160 @ 30fps  
make square    # 1080x1080 @ 30fps
make portrait  # 1080x1920 @ 30fps
```

### Custom Recording

```bash
# Override any setting on the fly
make video VIDEO_WIDTH=2560 VIDEO_HEIGHT=1440 VIDEO_FPS=60 VIDEO_DURATION=15

# Record with custom filename
make video VIDEO_FILENAME=my_artwork.mp4
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
VIDEO_FILENAME = output.mp4

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
├── src/
│   ├── ofApp.h          # Main application header
│   ├── ofApp.cpp        # Application logic with video recording
│   └── main.cpp         # Entry point
├── config.make          # Project configuration
├── Makefile            # Self-contained build system
├── addons.make         # openFrameworks addons
└── README.md           # This file
```

## 🎥 How Video Recording Works

1. **Frame Generation**: App runs in headless mode, saving each frame as PNG
2. **Automatic Encoding**: FFmpeg converts frame sequence to H.264 MP4
3. **Cleanup**: Temporary frames are automatically removed
4. **Output**: Final video saved to project directory

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
make clean-all       # Clean everything (build + frames + videos)
```

## 🔧 Advanced Usage

### Environment Variables

The app reads these environment variables (set by Makefile):

- `OUTPUT_TYPE` - "app" or "video"
- `VIDEO_WIDTH` - Frame width in pixels
- `VIDEO_HEIGHT` - Frame height in pixels  
- `VIDEO_FPS` - Frames per second
- `VIDEO_DURATION` - Recording duration in seconds

### Custom Animation

Replace the demo animation in `src/ofApp.cpp` `draw()` method:

```cpp
void ofApp::draw(){
    // Your creative code here
    ofSetColor(255, 100, 200);
    ofDrawCircle(mouseX, mouseY, 50);
    
    // Video recording happens automatically
    if (isRecording) {
        saveFrame();
        frameCount++;
    }
}
```

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