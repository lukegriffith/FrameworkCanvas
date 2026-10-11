#pragma once

#include "ofMain.h"
#include <memory>

// Per-frame context handed to a sketch. This is the whole contract between
// a sketch and the harness: sketches never read env vars or the clock directly.
struct Ctx {
	int   w = 0, h = 0;        // render size in pixels
	float t = 0;               // seconds since start. Record mode: frame / fps (deterministic)
	float progress = 0;        // t / duration, wrapped to [0,1). Use for seamless loops
	int   frame = 0;           // frame index, 0-based
	float fps = 30;
	float duration = 10;       // seconds
	uint32_t seed = 1;         // ofSeedRandom(seed) is called before setup()
	bool  recording = false;
	ofJson params;             // contents of sketches/<name>/params.json, or empty object
};

class Sketch {
public:
	virtual ~Sketch() = default;
	virtual void setup(const Ctx&) {}
	virtual void draw(const Ctx&) = 0;   // called inside an FBO sized w x h, origin top-left
	virtual void keyPressed(int key, const Ctx&) {}
	virtual void mousePressed(int x, int y, int button, const Ctx&) {}
};

// Each sketch translation unit defines exactly one of these.
std::unique_ptr<Sketch> makeSketch();
