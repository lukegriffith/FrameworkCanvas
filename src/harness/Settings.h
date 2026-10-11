#pragma once

#include "ofMain.h"

// All harness configuration, read once from environment variables
// (set by the Makefile). Sketches never see this; they get a Ctx.
struct Settings {
	std::string outputType = "app";   // "app" or "video"
	int width = 1920;
	int height = 1080;
	int fps = 30;
	int duration = 10;                // seconds
	int renderStart = 0;              // first frame to render/save (record mode)
	int renderEnd = -1;               // last frame, inclusive; -1 means totalFrames-1
	uint32_t seed = 1;
	std::string paramsFile;           // absolute path to params.json, may be empty
	std::string framesDir;            // absolute path for frame output, empty = bin/data/frames
	ofJson params;                    // parsed paramsFile, or empty object

	int totalFrames() const { return duration * fps; }

	static Settings fromEnv();
};
