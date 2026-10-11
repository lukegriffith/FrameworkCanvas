// Template sketch. Copy this directory to sketches/<name>/ and edit.
//
//   cp -r sketches/_template sketches/mything
//   make check SKETCH=mything     # compile
//   make sheet SKETCH=mything     # look at the whole animation as one image
//
// Rules:
//  - Animate from ctx.t (seconds) or ctx.progress (0..1 over the clip), never from
//    ofGetFrameNum() or the wall clock. That keeps app mode and video identical.
//  - progress wraps, so sin(ctx.progress * TWO_PI * k) with integer k loops seamlessly.
//  - Size everything from ctx.w / ctx.h so any FORMAT works (square, portrait, 4k).
//  - Use ofRandom/ofNoise freely: the harness seeds the RNG from SEED (default 1).
//  - Origin is top-left. draw() starts on a black, cleared canvas.

#include "Sketch.h"

class MySketch : public Sketch {
public:
	void setup(const Ctx& ctx) override {
		// One-time setup: load fonts/shaders, generate random data.
	}

	void draw(const Ctx& ctx) override {
		float unit = std::min(ctx.w, ctx.h);   // scale-independent length

		ofSetColor(255);
		ofDrawCircle(ctx.w / 2.0f, ctx.h / 2.0f,
		             unit * (0.2f + 0.05f * std::sin(ctx.progress * TWO_PI)));
	}
};

std::unique_ptr<Sketch> makeSketch() {
	return std::make_unique<MySketch>();
}
