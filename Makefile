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
# Default output names follow the sketch; files land in out/<sketch>/
ifndef VIDEO_FILENAME
	VIDEO_FILENAME = $(SKETCH_NAME)$(if $(FORMAT),_$(FORMAT)).mp4
endif
ifndef GIF_FILENAME
	GIF_FILENAME = $(SKETCH_NAME)$(if $(FORMAT),_$(FORMAT)).gif
endif

# FFmpeg path (adjust if ffmpeg is not in PATH)
ifndef FFMPEG
	FFMPEG = ffmpeg
endif

# Named output formats: name:WIDTHxHEIGHTxFPS.
# `make video FORMAT=instagram-story` (or the alias `make instagram-story`) sets
# VIDEO_WIDTH/HEIGHT/FPS from this table. Explicit VIDEO_* on the command line still win.
VIDEO_FORMATS := hd 4k square portrait instagram-feed instagram-story instagram-reel instagram-portrait
GIF_FORMATS := gif-small gif-web gif-square gif-instagram-feed gif-instagram-story
FORMATS := hd:1920x1080x30 4k:3840x2160x30 square:1080x1080x30 portrait:1080x1920x30 \
           instagram-feed:1080x1080x30 instagram-story:1080x1920x30 instagram-reel:1080x1920x30 \
           instagram-portrait:1080x1350x30 \
           gif-small:400x400x15 gif-web:800x600x20 gif-square:500x500x24 \
           gif-instagram-feed:1080x1080x24 gif-instagram-story:1080x1920x24
ifdef FORMAT
  FORMAT_SPEC := $(patsubst $(FORMAT):%,%,$(filter $(FORMAT):%,$(FORMATS)))
  ifeq ($(FORMAT_SPEC),)
    $(error unknown FORMAT '$(FORMAT)'. Known: $(subst :, ,$(foreach f,$(FORMATS),$(firstword $(subst :, ,$(f))))))
  endif
  VIDEO_WIDTH  := $(word 1,$(subst x, ,$(FORMAT_SPEC)))
  VIDEO_HEIGHT := $(word 2,$(subst x, ,$(FORMAT_SPEC)))
  VIDEO_FPS    := $(word 3,$(subst x, ,$(FORMAT_SPEC)))
endif

# Export variables so they're available to the compiled application
export OUTPUT_TYPE
export VIDEO_WIDTH
export VIDEO_HEIGHT
export VIDEO_FPS
export VIDEO_DURATION

################################################################################
# SKETCH SELECTION
# Each sketch lives in sketches/<name>/ and defines makeSketch() (see src/Sketch.h).
# Only the chosen sketch directory is compiled and linked alongside src/.
################################################################################
SKETCH ?= demo
# A sketch is either C++ (sketch.cpp, openFrameworks lane) or JavaScript (sketch.js, web/p5 lane).
HAS_CPP := $(wildcard sketches/$(SKETCH)/sketch.cpp)
HAS_JS  := $(wildcard sketches/$(SKETCH)/sketch.js)
ifneq ($(HAS_CPP),)
  ifneq ($(HAS_JS),)
    $(error sketches/$(SKETCH) has both sketch.cpp and sketch.js: keep only one)
  endif
  ENGINE := of
  FRAME_EXT := tga
else ifneq ($(HAS_JS),)
  ENGINE := web
  FRAME_EXT := png
else
  $(error sketch '$(SKETCH)' not found: expected sketches/$(SKETCH)/sketch.cpp or sketch.js)
endif
export SKETCH
# Sketches may be nested (e.g. _examples/particles); filenames use the last component
SKETCH_NAME = $(notdir $(SKETCH))

# Optional partial render: RENDER_START/RENDER_END are inclusive frame indices
RENDER_START ?= 0
export RENDER_START
override PROJECT_EXTERNAL_SOURCE_PATHS := $(abspath sketches/$(SKETCH))

# Optional per-sketch parameters, handed to the sketch as ctx.params without recompiling.
# Picks up sketches/<name>/params.json automatically; PARAMS_FILE=path overrides it.
# The path is made absolute because OF resolves relative paths against bin/data (see CLAUDE.md).
ifndef PARAMS_FILE
  ifneq ($(wildcard sketches/$(SKETCH)/params.json),)
    PARAMS_FILE := sketches/$(SKETCH)/params.json
  endif
endif
ifdef PARAMS_FILE
  override PARAMS_FILE := $(abspath $(PARAMS_FILE))
  export PARAMS_FILE
endif

# Sketch objects land directly in obj/<platform>/<config>/ (not under src/), and their
# .d files name the sketch source they came from. If that sketch is renamed or deleted,
# the stale .d makes every later build fail with "No rule to make target .../sketch.cpp".
# Clear them whenever the selected sketch changes. Cheap: only the sketch recompiles.
ifeq ($(ENGINE),of)
ifneq ($(shell cat obj/.sketch 2>/dev/null),$(SKETCH))
  SKETCH_OBJ_RESET := $(shell find obj -mindepth 3 -maxdepth 3 \( -name '*.o' -o -name '*.d' \) -delete 2>/dev/null; mkdir -p obj; printf '%s' '$(SKETCH)' > obj/.sketch)
endif
endif

# Per-sketch output: out/<sketch>/ holds frames and finished media
OUT_DIR := out/$(SKETCH)
override FRAMES_DIR := $(abspath $(OUT_DIR)/frames)
export FRAMES_DIR

override PLATFORM_RUN_COMMAND = echo "🚀 Starting interactive app..." && OUTPUT_TYPE=app ./bin/$(APPNAME).app/Contents/MacOS/$(APPNAME)

# call the project makefile!
# (guarded so that `make setup` can still run and give useful guidance even
#  when OF_ROOT isn't valid yet - see the setup target below)
ifneq ($(wildcard $(OF_ROOT)/libs/openFrameworksCompiled/project/makefileCommon/compile.project.mk),)
	include $(OF_ROOT)/libs/openFrameworksCompiled/project/makefileCommon/compile.project.mk
endif

################################################################################
# DIGITAL ART WORKFLOW TARGETS
################################################################################

.PHONY: video gif clean-frames clean-video clean-gif record art help setup check build frame sheet draft smoke setup-web test-web

# Default help target
help:
	@echo "FrameworkCanvas: write a sketch, look at it, ship it."
	@echo ""
	@echo "First time: make setup        (C++ lane: check/locate OF_ROOT)"
	@echo "            make setup-web    (web lane: install p5 + headless Chromium)"
	@echo ""
	@echo "Agent loop (SKETCH=<name> picks sketches/<name>/, default demo):"
	@echo "  make check            compile only; prints OK or filtered errors"
	@echo "  make smoke            fail if the output is blank"
	@echo "  make frame T=2.5      still at t=2.5s   -> out/<sketch>/frame_2.5.png"
	@echo "  make sheet N=8        contact sheet     -> out/<sketch>/sheet.png"
	@echo "  make draft            480p 15fps preview -> out/<sketch>/draft.mp4"
	@echo "  make video            final MP4         -> out/<sketch>/<sketch>.mp4"
	@echo "  make gif              final GIF         -> out/<sketch>/<sketch>.gif"
	@echo ""
	@echo "Interactive: make run   (press r to record)"
	@echo ""
	@echo "Options (any target):"
	@echo "  SKETCH=name           which sketch (default: $(SKETCH)); sketch.js = web lane, sketch.cpp = C++ lane"
	@echo "  FORMAT=name           $(subst :, ,$(foreach f,$(FORMATS),$(firstword $(subst :, ,$(f)))))"
	@echo "  VIDEO_WIDTH/HEIGHT/FPS/DURATION   current: $(VIDEO_WIDTH)x$(VIDEO_HEIGHT) @ $(VIDEO_FPS)fps, $(VIDEO_DURATION)s"
	@echo "  SEED=n                random seed (default 1)"
	@echo "  PARAMS_FILE=path      JSON passed to the sketch as ctx.params"
	@echo "  RENDER_START/END=n    render only this frame range"
	@echo ""
	@echo "Aliases: each FORMAT name is also a target, e.g. make instagram-story, make gif-web"
	@echo ""
	@echo "Cleanup: make clean-frames | clean-video | clean-gif | clean-all"

# Check that OF_ROOT (from config.make, or an env/CLI override) points at a
# real openFrameworks install, and if not, try to help find one. Safe to run
# any time - it never modifies config.make itself.
OF_MARKER := libs/openFrameworksCompiled/project/makefileCommon/compile.project.mk

setup:
	@bash -c ' \
	of_root="$(OF_ROOT)"; \
	marker="$(OF_MARKER)"; \
	if [ -f "$$of_root/$$marker" ]; then \
		echo "OF_ROOT looks good: $$of_root"; \
		exit 0; \
	fi; \
	echo "OF_ROOT is not set to a valid openFrameworks install."; \
	echo "  Currently: $$of_root"; \
	echo ""; \
	echo "Searching common install locations..."; \
	candidates=""; \
	for base in "$$HOME/Downloads" "$$HOME/openFrameworks" "$$HOME" "/usr/local/opt" "/opt"; do \
		[ -d "$$base" ] || continue; \
		for d in "$$base"/of_v* "$$base"/openFrameworks*; do \
			[ -e "$$d" ] || continue; \
			if [ -f "$$d/$$marker" ]; then \
				candidates="$$candidates $$d"; \
			fi; \
		done; \
	done; \
	count=$$(echo $$candidates | wc -w | tr -d "[:space:]"); \
	echo ""; \
	if [ "$$count" -eq 1 ]; then \
		found=$$(echo $$candidates | xargs); \
		echo "Found an openFrameworks install:"; \
		echo "  $$found"; \
		echo ""; \
		echo "Add/update this line in config.make:"; \
		echo "  OF_ROOT = $$found"; \
	else \
		if [ "$$count" -gt 1 ]; then \
			echo "Found multiple possible openFrameworks installs (ambiguous, pick one):"; \
			for c in $$candidates; do echo "  $$c"; done; \
			echo ""; \
		else \
			echo "Could not find an openFrameworks install automatically."; \
			echo ""; \
		fi; \
		echo "Download openFrameworks from https://openframeworks.cc/download/"; \
		echo "then set OF_ROOT in config.make to point at the extracted folder, e.g.:"; \
		echo "  OF_ROOT = /path/to/of_v0.12.1_osx_release"; \
	fi; \
	exit 1; \
	'

# Install the web engine (headless Chromium + p5). Safe to rerun; no-op when up to date.
setup-web:
	@command -v node >/dev/null || { echo "Node 20+ is required: brew install node"; exit 1; }
	@cd engines/web && if [ -f package-lock.json ]; then npm ci --no-audit --no-fund --loglevel=error; else npm install --no-audit --no-fund --loglevel=error; fi
	@cd engines/web && node -e "import('puppeteer').then(async p=>console.log('chromium:', await p.default.executablePath()))"

# Web engine self-tests (determinism, ctx contract, error handling)
test-web:
	@node engines/web/test/run.mjs

# Engine dispatch: produce frame_NNNNNN.$(FRAME_EXT) files in $(FRAMES_DIR).
#   $(call render_frames,WIDTH,HEIGHT,FPS,START,END)   (END=-1 means "to the end")
# Everything after this step (ffmpeg, sheets, smoke) is shared between engines.
ifeq ($(ENGINE),web)
BUILD_DEP :=
render_frames = { [ -d engines/web/node_modules ] || { echo "web engine not installed: run make setup-web"; exit 1; }; } && \
	FC_QUIET=1 node engines/web/render.mjs --sketch sketches/$(SKETCH)/sketch.js --frames-dir $(FRAMES_DIR) \
		--w $(1) --h $(2) --fps $(3) --duration $(VIDEO_DURATION) --start $(4) --end $(5) \
		--seed $(or $(SEED),1) $(if $(PARAMS_FILE),--params $(PARAMS_FILE))
else
BUILD_DEP := build
render_frames = OUTPUT_TYPE=video VIDEO_WIDTH=$(1) VIDEO_HEIGHT=$(2) VIDEO_FPS=$(3) RENDER_START=$(4) RENDER_END=$(5) \
	./bin/$(APPNAME).app/Contents/MacOS/$(APPNAME) >/dev/null
endif

# Quiet build: prints nothing on success, filtered compiler errors on failure.
# All media targets depend on this instead of Release so their output stays short
# (OF's Release target prints hundreds of lines on every run, even when up to date).
build:
	@out=$$($(MAKE) Release 2>&1); rc=$$?; \
	if [ $$rc -ne 0 ]; then \
		echo "$$out" | grep -E -A3 "error:|Undefined symbols|ld: |\*\*\*" | head -40; \
		exit 1; \
	fi

# Fast edit loop. Prints OK on success, a short error otherwise.
#   OF lane:  compile only.
#   Web lane: syntax check, then render frame 0 at low resolution (there is no compiler,
#             so actually running the sketch is the real check).
ifeq ($(ENGINE),web)
check:
	@out=$$(node --check sketches/$(SKETCH)/sketch.js 2>&1) || { echo "$$out" | grep -v '^    at \|^Node.js\|^$$' | head -8; exit 1; }
	@rm -rf $(FRAMES_DIR); mkdir -p $(FRAMES_DIR)
	@$(call render_frames,$(SMOKE_WIDTH),$(SMOKE_HEIGHT),$(SMOKE_FPS),0,0) || { rm -rf $(FRAMES_DIR); exit 1; }
	@rm -rf $(FRAMES_DIR)
	@echo "OK"
else
check: build
	@echo "OK"
endif

# Render a single still at time T (seconds) to out/<sketch>/frame_<T>.png
T ?= 0
frame: $(BUILD_DEP)
	@F=$$(awk 'BEGIN{printf "%d", $(T)*$(VIDEO_FPS)+0.5}'); \
	TOTAL=$$(( $(VIDEO_DURATION) * $(VIDEO_FPS) )); \
	if [ $$F -ge $$TOTAL ]; then echo "T=$(T) is past the end ($(VIDEO_DURATION)s)"; exit 1; fi; \
	rm -rf $(FRAMES_DIR); mkdir -p $(FRAMES_DIR); \
	$(call render_frames,$(VIDEO_WIDTH),$(VIDEO_HEIGHT),$(VIDEO_FPS),$$F,$$F) || exit 1; \
	$(FFMPEG) -y -v error -i $(FRAMES_DIR)/frame_$$(printf %06d $$F).$(FRAME_EXT) -frames:v 1 $(OUT_DIR)/frame_$(T).png || exit 1; \
	rm -rf $(FRAMES_DIR); \
	echo "✅ Frame saved as $(OUT_DIR)/frame_$(T).png (frame $$F)"

# Contact sheet: N frames spread evenly across the clip, 4 columns, each labelled
# with its time. One image shows the whole animation. Output: out/<sketch>/sheet.png
N ?= 8
SHEET_COLS ?= 4
SHEET_CELL_WIDTH ?= 480
SHEET_FONT ?= /System/Library/Fonts/Menlo.ttc
sheet: $(BUILD_DEP)
	@rm -rf $(FRAMES_DIR); mkdir -p $(FRAMES_DIR); \
	$(call render_frames,$(VIDEO_WIDTH),$(VIDEO_HEIGHT),$(VIDEO_FPS),0,-1) || exit 1; \
	TOTAL=$$(( $(VIDEO_DURATION) * $(VIDEO_FPS) )); \
	SEL=""; k=0; while [ $$k -lt $(N) ]; do SEL="$$SEL+eq(n,$$(( k * TOTAL / $(N) )))"; k=$$(( k + 1 )); done; \
	ROWS=$$(( ($(N) + $(SHEET_COLS) - 1) / $(SHEET_COLS) )); \
	LABEL=""; \
	if [ -f "$(SHEET_FONT)" ]; then \
		LABEL="drawtext=fontfile=$(SHEET_FONT):text='%{pts\\:hms}':x=12:y=12:fontsize=$$(( $(VIDEO_WIDTH) / 20 )):fontcolor=white:box=1:boxcolor=black@0.6:boxborderw=6,"; \
	fi; \
	$(FFMPEG) -y -v error -framerate $(VIDEO_FPS) -i $(FRAMES_DIR)/frame_%06d.$(FRAME_EXT) \
		-vf "select='0$$SEL',$${LABEL}scale=$(SHEET_CELL_WIDTH):-2,tile=$(SHEET_COLS)x$$ROWS:padding=4:color=gray" \
		-frames:v 1 -update 1 $(OUT_DIR)/sheet.png || exit 1; \
	rm -rf $(FRAMES_DIR); \
	echo "✅ Sheet saved as $(OUT_DIR)/sheet.png ($(N) frames across $(VIDEO_DURATION)s)"

# Fast low-res preview: 480px wide, 15fps, same aspect and SAME DURATION as the
# real render. Duration is kept because ctx.progress is relative to it; shortening
# it would squeeze the whole animation into fewer seconds.
DRAFT_WIDTH ?= 480
DRAFT_FPS ?= 15
DRAFT_HEIGHT = $(shell awk 'BEGIN{printf "%d", int($(DRAFT_WIDTH)*$(VIDEO_HEIGHT)/$(VIDEO_WIDTH)/2+0.5)*2}')
draft:
	@$(MAKE) video VIDEO_WIDTH=$(DRAFT_WIDTH) VIDEO_HEIGHT=$(DRAFT_HEIGHT) VIDEO_FPS=$(DRAFT_FPS) VIDEO_FILENAME=draft.mp4

# Smoke test: render tiny, sample first/middle/last frames, fail if ALL are one flat
# colour (empty draw(), crash before drawing, everything off-screen). A single flat
# frame only warns, since fade-ins from black are legitimate.
SMOKE_WIDTH ?= 320
SMOKE_FPS ?= 10
SMOKE_HEIGHT = $(shell awk 'BEGIN{printf "%d", int($(SMOKE_WIDTH)*$(VIDEO_HEIGHT)/$(VIDEO_WIDTH)/2+0.5)*2}')
smoke: $(BUILD_DEP)
	@rm -rf $(FRAMES_DIR); mkdir -p $(FRAMES_DIR); \
	$(call render_frames,$(SMOKE_WIDTH),$(SMOKE_HEIGHT),$(SMOKE_FPS),0,-1) || { echo "smoke: render failed"; exit 1; }; \
	TOTAL=$$(( $(VIDEO_DURATION) * $(SMOKE_FPS) )); \
	flat=0; checked=0; \
	for F in 0 $$(( TOTAL / 2 )) $$(( TOTAL - 1 )); do \
		FILE=$(FRAMES_DIR)/frame_$$(printf %06d $$F).$(FRAME_EXT); \
		[ -f "$$FILE" ] || { echo "smoke: frame $$F was not written"; rm -rf $(FRAMES_DIR); exit 1; }; \
		STATS=$$($(FFMPEG) -v error -i $$FILE -vf "signalstats,metadata=print:file=-" -f null - 2>&1); \
		RANGE=0; \
		for C in Y U V; do \
			LO=$$(echo "$$STATS" | sed -n "s/.*signalstats.$${C}MIN=//p"); HI=$$(echo "$$STATS" | sed -n "s/.*signalstats.$${C}MAX=//p"); \
			[ "$$LO" != "$$HI" ] && RANGE=1; \
		done; \
		checked=$$(( checked + 1 )); \
		if [ $$RANGE -eq 0 ]; then flat=$$(( flat + 1 )); echo "smoke: warning: frame $$F is a single flat colour"; fi; \
	done; \
	rm -rf $(FRAMES_DIR); \
	if [ $$flat -eq $$checked ]; then echo "smoke: FAIL: all sampled frames are blank"; exit 1; fi; \
	echo "✅ smoke OK ($$(( checked - flat ))/$$checked sampled frames have content)"

# Build and run for video recording
video: $(BUILD_DEP)
	@echo "🎬 Starting video recording..."
	@echo "   Resolution: $(VIDEO_WIDTH)x$(VIDEO_HEIGHT)"
	@echo "   Framerate:  $(VIDEO_FPS) fps"
	@echo "   Duration:   $(VIDEO_DURATION) seconds"
	@echo "   Output:     $(VIDEO_FILENAME)"
	@echo ""
	@rm -rf $(FRAMES_DIR); mkdir -p $(FRAMES_DIR)
	@$(call render_frames,$(VIDEO_WIDTH),$(VIDEO_HEIGHT),$(VIDEO_FPS),$(RENDER_START),$(or $(RENDER_END),-1))
	@echo ""
	@echo "🔧 Creating MP4 with ffmpeg..."
	@$(FFMPEG) -y -framerate $(VIDEO_FPS) -start_number $(RENDER_START) -i $(FRAMES_DIR)/frame_%06d.$(FRAME_EXT) -c:v libx264 -pix_fmt yuv420p -crf 18 -color_primaries bt709 -color_trc bt709 -colorspace bt709 -color_range tv -x264-params "colorprim=bt709:transfer=bt709:colormatrix=bt709:fullrange=off" $(OUT_DIR)/$(VIDEO_FILENAME)
	@echo "✅ Video saved as $(OUT_DIR)/$(VIDEO_FILENAME)"
	@$(MAKE) clean-frames

# Build and run for GIF recording
gif: $(BUILD_DEP)
	@echo "🎨 Starting GIF recording..."
	@echo "   Resolution: $(VIDEO_WIDTH)x$(VIDEO_HEIGHT)"
	@echo "   Framerate:  $(VIDEO_FPS) fps"
	@echo "   Duration:   $(VIDEO_DURATION) seconds"
	@echo "   Output:     $(GIF_FILENAME)"
	@echo ""
	@rm -rf $(FRAMES_DIR); mkdir -p $(FRAMES_DIR)
	@$(call render_frames,$(VIDEO_WIDTH),$(VIDEO_HEIGHT),$(VIDEO_FPS),$(RENDER_START),$(or $(RENDER_END),-1))
	@echo ""
	@echo "🔧 Creating GIF with ffmpeg..."
	@$(FFMPEG) -y -framerate $(VIDEO_FPS) -start_number $(RENDER_START) -i $(FRAMES_DIR)/frame_%06d.$(FRAME_EXT) -vf "fps=$(VIDEO_FPS),scale=$(VIDEO_WIDTH):$(VIDEO_HEIGHT):flags=lanczos,palettegen" -t $(VIDEO_DURATION) $(OUT_DIR)/palette.png
	@$(FFMPEG) -y -framerate $(VIDEO_FPS) -start_number $(RENDER_START) -i $(FRAMES_DIR)/frame_%06d.$(FRAME_EXT) -i $(OUT_DIR)/palette.png -filter_complex "fps=$(VIDEO_FPS),scale=$(VIDEO_WIDTH):$(VIDEO_HEIGHT):flags=lanczos[x];[x][1:v]paletteuse" -t $(VIDEO_DURATION) $(OUT_DIR)/$(GIF_FILENAME)
	@rm -f $(OUT_DIR)/palette.png
	@echo "✅ GIF saved as $(OUT_DIR)/$(GIF_FILENAME)"
	@$(MAKE) clean-frames

# Alias for artists who prefer 'make art'
art: video

# Alternative: record with custom settings
record:
	@echo "🎥 Recording with settings: $(VIDEO_WIDTH)x$(VIDEO_HEIGHT) @ $(VIDEO_FPS)fps for $(VIDEO_DURATION)s"
	@$(MAKE) video

# Run the interactive app.
# OF already defines `run`; defining it again triggers "overriding commands"
# warnings, so instead we supply the command OF's `run` executes, and only add
# the Release prerequisite here (no recipe).
run: Release

# Format aliases: `make instagram-story` == `make video FORMAT=instagram-story`
.PHONY: $(VIDEO_FORMATS) $(GIF_FORMATS) instagram
$(VIDEO_FORMATS):
	@$(MAKE) video FORMAT=$@

$(GIF_FORMATS):
	@$(MAKE) gif FORMAT=$@

# Alias for most common Instagram format
instagram: instagram-feed

# Clean frame files
clean-frames:
	@echo "🧹 Cleaning frame files..."
	@rm -rf $(FRAMES_DIR)

# Clean video files
clean-video:
	@echo "🧹 Cleaning video files..."
	@rm -f out/*/*.mp4

# Clean GIF files
clean-gif:
	@echo "🧹 Cleaning GIF files..."
	@rm -f out/*/*.gif

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
