// SIGNAL: kinetic typography. Letters rise in a travelling wave, shift colour, settle.
#include "Sketch.h"

class SignalSketch : public Sketch {
	ofTrueTypeFont font;
	std::string text = "SIGNAL";
	std::vector<float> widths;

public:
	void setup(const Ctx& ctx) override {
		int size = (int)(ctx.w * 0.165f);
		font.load(OF_TTF_SANS, size, true, false, true);
		for (char c : text) widths.push_back(font.stringWidth(std::string(1, c)));
	}

	void draw(const Ctx& ctx) override {
		ofBackground(8, 9, 16);
		float unit = std::min(ctx.w, ctx.h);
		int n = (int)text.size();

		float tracking = ctx.w * 0.012f;
		float total = tracking * (n - 1);
		for (float w : widths) total += w;
		float x = (ctx.w - total) / 2.0f;
		float baseline = ctx.h / 2.0f + font.stringHeight("S") * 0.5f;

		// Faint horizon line the letters rise from.
		ofSetColor(255, 30);
		ofDrawRectangle(ctx.w * 0.1f, baseline + unit * 0.02f, ctx.w * 0.8f, 2);

		for (int i = 0; i < n; i++) {
			// Two wave cycles per loop (integer), each letter delayed along the word.
			float ph = ctx.progress * 2.0f - i * 0.09f;
			float s = 0.5f - 0.5f * std::cos(ph * TWO_PI);   // 0..1
			// Sharp rise, long settle: s^3 is near 0 most of the time.
			float lift = s * s * s;
			float rise = lift * unit * 0.12f;

			// Hue travels with the wave and loops.
			float hue = std::fmod(150.0f + 70.0f * lift + 255.0f * ctx.progress * 1.0f + i * 12.0f, 255.0f);
			float sat = 90.0f + 110.0f * lift;
			ofColor c = ofColor::fromHsb(hue, sat, 255);

			std::string ch(1, text[i]);
			float y = baseline - rise;
			ofSetColor(c);
			font.drawString(ch, x, y);

			x += widths[i] + tracking;
		}
	}
};

std::unique_ptr<Sketch> makeSketch() { return std::make_unique<SignalSketch>(); }
