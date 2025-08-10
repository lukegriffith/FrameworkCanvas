# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

FrameworkCanvas is an openFrameworks digital art template that provides dual-mode operation: interactive app development and automated video/GIF generation. It's designed as a starting template for digital art projects with a streamlined make-based workflow.

## Core Architecture

### Application Structure
- `src/ofApp.h` - Main application header with video recording functionality
- `src/ofApp.cpp` - Core application logic with dual-mode support (interactive/video)
- `src/main.cpp` - Entry point with basic window setup
- `Makefile` - Self-contained build system with media generation workflow
- `config.make` - Project configuration with openFrameworks path and video settings

### Key Components
- **Dual Mode System**: Uses `OUTPUT_TYPE` environment variable to switch between "app" (interactive) and "video" (recording) modes
- **Video Recording**: Frame-by-frame PNG export with FFmpeg conversion to MP4/GIF
- **Environment-based Configuration**: Video settings passed through environment variables from Makefile to C++ application

## Development Commands

### Build and Run
```bash
make              # Build interactive .app
make run          # Build and run interactive app
make Release      # Build release version (used internally)
```

### Media Generation
```bash
make video        # Create MP4 video with current settings
make gif          # Create animated GIF with current settings  
make art          # Alias for 'make video'
```

### Presets
```bash
# Video formats
make hd           # 1920x1080 @ 30fps
make 4k           # 3840x2160 @ 30fps
make square       # 1080x1080 @ 30fps
make portrait     # 1080x1920 @ 30fps

# Instagram optimized
make instagram           # 1080x1080 feed post (most common)
make instagram-story     # 1080x1920 story/reel format
make instagram-reel      # 1080x1920 reel format
make instagram-portrait  # 1080x1350 portrait post

# GIF presets (web optimized)
make gif-small           # 400x400 @ 15fps
make gif-web             # 800x600 @ 20fps
make gif-square          # 500x500 @ 24fps
make gif-instagram-feed  # 1080x1080 @ 24fps
make gif-instagram-story # 1080x1920 @ 24fps
```

### Cleanup
```bash
make clean-frames    # Remove temporary frame files
make clean-video     # Remove generated MP4s
make clean-gif       # Remove generated GIF files
make clean-all       # Clean everything (build + frames + videos + gifs)
```

### Custom Settings
Override any video setting on the fly:
```bash
make video VIDEO_WIDTH=2560 VIDEO_HEIGHT=1440 VIDEO_FPS=60 VIDEO_DURATION=15
make gif VIDEO_WIDTH=600 VIDEO_HEIGHT=400 VIDEO_FPS=12 GIF_FILENAME=custom.gif
```

## Configuration

### Video Settings
Configure defaults in `config.make`:
- `OUTPUT_TYPE` - "app" or "video" 
- `VIDEO_WIDTH`, `VIDEO_HEIGHT` - Output resolution
- `VIDEO_FPS` - Frame rate
- `VIDEO_DURATION` - Recording length in seconds
- `VIDEO_FILENAME`, `GIF_FILENAME` - Output filenames
- `FFMPEG` - FFmpeg executable path

### openFrameworks Setup
- `OF_ROOT` in `config.make` must point to openFrameworks installation
- Currently set to: `/Users/lukegriffith/Downloads/of_v0.12.1_osx_release`

## Architecture Notes

### Video Recording Flow
1. Makefile sets environment variables and builds app in Release mode
2. App reads environment variables in `ofApp::loadVideoSettings()`
3. In video mode: app runs headless, saves frames to `bin/data/frames/`
4. FFmpeg converts frames to final MP4/GIF format
5. Temporary frames are automatically cleaned up

### Interactive Mode Features
- Real-time preview with 60fps
- Press 'r' to start manual recording
- FPS counter and frame information display
- Full mouse/keyboard event handling available

## Dependencies

- **openFrameworks 0.12.1+** - Core framework
- **Xcode Command Line Tools** - For compilation
- **FFmpeg** - For video/GIF generation (`brew install ffmpeg`)
- **C++17 compatible compiler**

## Template Usage

This project is designed to be forked and customized. Replace the demo animation in `ofApp::draw()` with your creative code. The video recording system will automatically capture your animations.

## Commit Message Guidelines

Use conventional commits format:
- `feat:` for new features
- `fix:` for bug fixes  
- `docs:` for documentation changes
- `refactor:` for code refactoring
- `style:` for formatting changes
- `test:` for adding tests
- `chore:` for maintenance tasks