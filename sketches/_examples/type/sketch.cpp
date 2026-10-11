// Example: animated typography with a system font, seamless loop.
//
// Pattern to copy: ofTrueTypeFont::load(OF_TTF_SANS, size) uses the platform's
// default sans font, so nothing needs bundling. To use your own font, put the
// .ttf in bin/data/ and load("MyFont.ttf", size): paths are relative to bin/data.
//
// Font size is chosen in setup() from ctx.h, so text scales with the format. The
// bitmap atlas is built once; never call load() inside draw().

#include "Sketch.h"

class TypeSketch : public Sketch {
	ofTrueTypeFont font;
	std::string text = "LOOP";
	std::vector<float> widths;

public:
	void setup(const Ctx& ctx) override {
		int size = (int)(std::min(ctx.w, ctx.h) * 0.28f);
		font.load(OF_TTF_SANS, size, true, false, true);   // antialiased, contours for outlines
		for (char c : text) widths.push_back(font.stringWidth(std::string(1, c)));
	}

	void draw(const Ctx& ctx) override {
		ofBackground(18, 18, 24);
		float unit = std::min(ctx.w, ctx.h);

		float total = 0;
		for (float w : widths) total += w;
		float tracking = unit * 0.02f;
		total += tracking * (text.size() - 1);

		float x = (ctx.w - total) / 2.0f;
		float baseline = ctx.h / 2.0f + font.getLineHeight() * 0.3f;

		for (size_t i = 0; i < text.size(); i++) {
			// Each letter bobs with a phase offset; integer cycle count keeps the loop seamless.
			float ph = ctx.progress + i / (float)text.size() * 0.5f;
			float bob = std::sin(ph * TWO_PI * 2) * unit * 0.05f;
			float hue = std::fmod(ph * 255.0f * 1.0f, 255.0f);

			// Soft shadow, then the letter.
			ofSetColor(0, 0, 0, 120);
			font.drawString(std::string(1, text[i]), x + unit * 0.01f, baseline + bob + unit * 0.01f);
			ofSetColor(ofColor::fromHsb(hue, 140, 255));
			font.drawString(std::string(1, text[i]), x, baseline + bob);

			x += widths[i] + tracking;
		}

		// A thin progress bar: handy when judging loop length in a sheet.
		ofSetColor(255, 60);
		ofDrawRectangle(0, ctx.h - unit * 0.01f, ctx.w * ctx.progress, unit * 0.01f);
	}
};

std::unique_ptr<Sketch> makeSketch() { return std::make_unique<TypeSketch>(); }
