################################################################################
# FrameworkCanvas - openFrameworks Digital Art Template
# Complete self-contained Makefile for interactive apps and MP4 generation
################################################################################

# Attempt to load a config.make file.
# If none is found, project defaults in config.project.make will be used.
ifneq ($(wildcard config.make),)
	include config.make
endif

# make sure the the OF_ROOT location is defined
ifndef OF_ROOT
	OF_ROOT=../../..
endif

################################################################################
# VIDEO RECORDING CONFIGURATION
# Configure these settings for video output, or override in config.make
################################################################################

# OUTPUT_TYPE: Choose output type: "app" or "video" 
#   app    = Build .app for interactive use  
#   video  = Build for video recording (saves frames and creates MP4)
ifndef OUTPUT_TYPE
	OUTPUT_TYPE = app
endif

# Video settings (used when OUTPUT_TYPE = video)
ifndef VIDEO_WIDTH
	VIDEO_WIDTH = 1920
endif
ifndef VIDEO_HEIGHT  
	VIDEO_HEIGHT = 1080
endif
ifndef VIDEO_FPS
	VIDEO_FPS = 30
endif
ifndef VIDEO_DURATION
	VIDEO_DURATION = 10
endif
ifndef VIDEO_FILENAME
	VIDEO_FILENAME = output.mp4
endif
ifndef GIF_FILENAME
	GIF_FILENAME = output.gif
endif

# FFmpeg path (adjust if ffmpeg is not in PATH)
ifndef FFMPEG
	FFMPEG = ffmpeg
endif

# Export variables so they're available to the compiled application
export OUTPUT_TYPE
export VIDEO_WIDTH
export VIDEO_HEIGHT
export VIDEO_FPS
export VIDEO_DURATION

# call the project makefile!
include $(OF_ROOT)/libs/openFrameworksCompiled/project/makefileCommon/compile.project.mk

################################################################################
# DIGITAL ART WORKFLOW TARGETS
################################################################################

.PHONY: video gif clean-frames clean-video clean-gif record art help

# Default help target
help:
	@echo "FrameworkCanvas - Digital Art Template"
	@echo "======================================"
	@echo ""
	@echo "Basic Usage:"
	@echo "  make           - Build interactive .app"  
	@echo "  make run       - Build and run interactive .app"
	@echo "  make video     - Create MP4 video (uses settings below)"
	@echo "  make gif       - Create animated GIF (smaller file, web-friendly)"
	@echo "  make art       - Alias for 'make video'"
	@echo ""
	@echo "Video Settings (override in config.make):"
	@echo "  OUTPUT_TYPE    = $(OUTPUT_TYPE)"
	@echo "  VIDEO_WIDTH    = $(VIDEO_WIDTH)" 
	@echo "  VIDEO_HEIGHT   = $(VIDEO_HEIGHT)"
	@echo "  VIDEO_FPS      = $(VIDEO_FPS)"
	@echo "  VIDEO_DURATION = $(VIDEO_DURATION)s"
	@echo "  VIDEO_FILENAME = $(VIDEO_FILENAME)"
	@echo "  GIF_FILENAME   = $(GIF_FILENAME)"
	@echo ""
	@echo "Instagram Presets:"
	@echo "  make instagram       - 1080x1080 feed post (default)"
	@echo "  make instagram-story - 1080x1920 story/reel format"
	@echo "  make instagram-reel  - 1080x1920 reel format"
	@echo ""
	@echo "Custom Recording:"
	@echo "  make record VIDEO_WIDTH=3840 VIDEO_HEIGHT=2160 VIDEO_FPS=60"
	@echo "  make gif VIDEO_WIDTH=800 VIDEO_HEIGHT=600 GIF_FILENAME=demo.gif"
	@echo ""
	@echo "Cleanup:"
	@echo "  make clean-frames    - Remove temporary frame files"
	@echo "  make clean-video     - Remove generated MP4 files"
	@echo "  make clean-gif       - Remove generated GIF files" 
	@echo "  make clean-all       - Clean everything (build + frames + videos + gifs)"

# Build and run for video recording
video: Release
	@echo "🎬 Starting video recording..."
	@echo "   Resolution: $(VIDEO_WIDTH)x$(VIDEO_HEIGHT)"
	@echo "   Framerate:  $(VIDEO_FPS) fps"
	@echo "   Duration:   $(VIDEO_DURATION) seconds"
	@echo "   Output:     $(VIDEO_FILENAME)"
	@echo ""
	@mkdir -p bin/data/frames
	@export OUTPUT_TYPE=video VIDEO_WIDTH=$(VIDEO_WIDTH) VIDEO_HEIGHT=$(VIDEO_HEIGHT) VIDEO_FPS=$(VIDEO_FPS) VIDEO_DURATION=$(VIDEO_DURATION) && ./bin/$(APPNAME).app/Contents/MacOS/$(APPNAME)
	@echo ""
	@echo "🔧 Creating MP4 with ffmpeg..."
	@$(FFMPEG) -y -framerate $(VIDEO_FPS) -i bin/data/frames/frame_%06d.png -c:v libx264 -pix_fmt yuv420p -crf 18 $(VIDEO_FILENAME)
	@echo "✅ Video saved as $(VIDEO_FILENAME)"
	@$(MAKE) clean-frames

# Build and run for GIF recording
gif: Release
	@echo "🎨 Starting GIF recording..."
	@echo "   Resolution: $(VIDEO_WIDTH)x$(VIDEO_HEIGHT)"
	@echo "   Framerate:  $(VIDEO_FPS) fps"
	@echo "   Duration:   $(VIDEO_DURATION) seconds"
	@echo "   Output:     $(GIF_FILENAME)"
	@echo ""
	@mkdir -p bin/data/frames
	@export OUTPUT_TYPE=video VIDEO_WIDTH=$(VIDEO_WIDTH) VIDEO_HEIGHT=$(VIDEO_HEIGHT) VIDEO_FPS=$(VIDEO_FPS) VIDEO_DURATION=$(VIDEO_DURATION) && ./bin/$(APPNAME).app/Contents/MacOS/$(APPNAME)
	@echo ""
	@echo "🔧 Creating GIF with ffmpeg..."
	@$(FFMPEG) -y -framerate $(VIDEO_FPS) -i bin/data/frames/frame_%06d.png -vf "fps=$(VIDEO_FPS),scale=$(VIDEO_WIDTH):$(VIDEO_HEIGHT):flags=lanczos,palettegen" -t $(VIDEO_DURATION) /tmp/palette.png
	@$(FFMPEG) -y -framerate $(VIDEO_FPS) -i bin/data/frames/frame_%06d.png -i /tmp/palette.png -filter_complex "fps=$(VIDEO_FPS),scale=$(VIDEO_WIDTH):$(VIDEO_HEIGHT):flags=lanczos[x];[x][1:v]paletteuse" -t $(VIDEO_DURATION) $(GIF_FILENAME)
	@rm -f /tmp/palette.png
	@echo "✅ GIF saved as $(GIF_FILENAME)"
	@$(MAKE) clean-frames

# Alias for artists who prefer 'make art'
art: video

# Alternative: record with custom settings
record:
	@echo "🎥 Recording with settings: $(VIDEO_WIDTH)x$(VIDEO_HEIGHT) @ $(VIDEO_FPS)fps for $(VIDEO_DURATION)s"
	@$(MAKE) video

# Run the interactive app
run: Release
	@echo "🚀 Starting interactive app..."
	@export OUTPUT_TYPE=app && ./bin/$(APPNAME).app/Contents/MacOS/$(APPNAME)

# Quick video with different settings
hd: 
	@$(MAKE) video VIDEO_WIDTH=1920 VIDEO_HEIGHT=1080 VIDEO_FPS=30

4k:
	@$(MAKE) video VIDEO_WIDTH=3840 VIDEO_HEIGHT=2160 VIDEO_FPS=30

portrait:
	@$(MAKE) video VIDEO_WIDTH=1080 VIDEO_HEIGHT=1920 VIDEO_FPS=30

square:
	@$(MAKE) video VIDEO_WIDTH=1080 VIDEO_HEIGHT=1080 VIDEO_FPS=30

# Instagram optimized presets  
instagram-feed:
	@$(MAKE) video VIDEO_WIDTH=1080 VIDEO_HEIGHT=1080 VIDEO_FPS=30 VIDEO_FILENAME=instagram_feed.mp4

instagram-story:
	@$(MAKE) video VIDEO_WIDTH=1080 VIDEO_HEIGHT=1920 VIDEO_FPS=30 VIDEO_FILENAME=instagram_story.mp4

instagram-reel:
	@$(MAKE) video VIDEO_WIDTH=1080 VIDEO_HEIGHT=1920 VIDEO_FPS=30 VIDEO_FILENAME=instagram_reel.mp4

instagram-portrait:
	@$(MAKE) video VIDEO_WIDTH=1080 VIDEO_HEIGHT=1350 VIDEO_FPS=30 VIDEO_FILENAME=instagram_portrait.mp4

# Alias for most common Instagram format
instagram: instagram-feed

# Quick GIF presets (optimized for web sharing)
gif-small:
	@$(MAKE) gif VIDEO_WIDTH=400 VIDEO_HEIGHT=400 VIDEO_FPS=15 GIF_FILENAME=small.gif

gif-web:
	@$(MAKE) gif VIDEO_WIDTH=800 VIDEO_HEIGHT=600 VIDEO_FPS=20 GIF_FILENAME=web.gif

gif-square:
	@$(MAKE) gif VIDEO_WIDTH=500 VIDEO_HEIGHT=500 VIDEO_FPS=24 GIF_FILENAME=square.gif

# Instagram GIF presets
gif-instagram-feed:
	@$(MAKE) gif VIDEO_WIDTH=1080 VIDEO_HEIGHT=1080 VIDEO_FPS=24 GIF_FILENAME=instagram_feed.gif

gif-instagram-story:
	@$(MAKE) gif VIDEO_WIDTH=1080 VIDEO_HEIGHT=1920 VIDEO_FPS=24 GIF_FILENAME=instagram_story.gif

# Clean frame files
clean-frames:
	@echo "🧹 Cleaning frame files..."
	@rm -rf bin/data/frames/

# Clean video files
clean-video:
	@echo "🧹 Cleaning video files..."
	@rm -f *.mp4

# Clean GIF files
clean-gif:
	@echo "🧹 Cleaning GIF files..."
	@rm -f *.gif

# Clean everything including frames, videos, and GIFs
clean-all: clean clean-frames clean-video clean-gif
	@echo "🧹 Everything cleaned!"

################################################################################
# PROJECT INFO
################################################################################

info:
	@echo "FrameworkCanvas v1.0"
	@echo "Project: $(APPNAME)"
	@echo "OF Root: $(OF_ROOT)"
	@echo "Output Type: $(OUTPUT_TYPE)"
	@echo "Platform: $(PLATFORM_OS)"
