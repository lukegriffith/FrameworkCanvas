#include "Sketch.h"

// Demo: rotating circles. One full rotation per clip, 3 pulses per clip,
// so the loop is seamless. Everything is driven by ctx.progress (0..1).
class DemoSketch : public Sketch {
public:
	void draw(const Ctx& ctx) override {
		float scale = std::min(ctx.w, ctx.h) / 540.0f;   // design size 540px short side

		ofPushMatrix();
		ofTranslate(ctx.w / 2.0f, ctx.h / 2.0f);
		ofScale(scale);
		ofRotateRad(ctx.progress * TWO_PI);

		ofSetColor(255, 100, 100);
		ofDrawCircle(0, 0, 100 + 50 * std::sin(ctx.progress * TWO_PI * 3));

		ofSetColor(100, 255, 100);
		ofDrawCircle(150, 0, 30);

		ofSetColor(100, 100, 255);
		ofDrawCircle(-150, 0, 30);
		ofPopMatrix();
	}
};

std::unique_ptr<Sketch> makeSketch() {
	return std::make_unique<DemoSketch>();
}
