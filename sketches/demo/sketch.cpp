#include "Sketch.h"

// Demo: rotating circles. By default one full rotation and 3 pulses per clip, so the
// loop is seamless. Everything is driven by ctx.progress (0..1).
//
// Tunable without recompiling: edit sketches/demo/params.json (read via ctx.params).
// Keep "turns" and "pulses" whole numbers or the loop will have a visible jump.
class DemoSketch : public Sketch {
public:
	void draw(const Ctx& ctx) override {
		float scale = std::min(ctx.w, ctx.h) / 540.0f;   // design size 540px short side

		ofPushMatrix();
		ofTranslate(ctx.w / 2.0f, ctx.h / 2.0f);
		ofScale(scale);
		ofRotateRad(ctx.progress * TWO_PI * ctx.params.value("turns", 1));

		ofSetColor(ofColor::fromHsb(ctx.params.value("hue", 0), 155, 255));   // hue 0 = the original coral
		ofDrawCircle(0, 0, 100 + 50 * std::sin(ctx.progress * TWO_PI * ctx.params.value("pulses", 3)));

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
