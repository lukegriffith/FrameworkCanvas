// B5: 14x14 data grid, dots ripple outward from the centre as a travelling wave.
#include "Sketch.h"

class GridSketch : public Sketch {
public:
	void draw(const Ctx& ctx) override {
		const int N = 14;
		float unit = std::min(ctx.w, ctx.h);
		ofBackground(246, 244, 239);

		float margin = unit * 0.11f;
		float cell = (unit - 2 * margin) / N;
		float ox = (ctx.w - cell * N) * 0.5f;
		float oy = (ctx.h - cell * N) * 0.5f;

		ofColor a(20, 150, 160);   // teal
		ofColor b(240, 90, 70);    // coral
		ofColor neutral(214, 211, 204);

		for (int j = 0; j < N; j++) {
			for (int i = 0; i < N; i++) {
				float dx = i - (N - 1) * 0.5f, dy = j - (N - 1) * 0.5f;
				float d = std::sqrt(dx * dx + dy * dy);
				// 2 integer cycles in time, outward travel: loops exactly
				float ph = TWO_PI * (ctx.progress * 2.0f - d / 5.0f);
				float s = std::sin(ph);
				float k = 0.5f + 0.5f * s;               // 0..1
				float r = cell * (0.12f + 0.36f * k);

				ofColor c;
				float m = std::abs(s);
				ofColor accent = s >= 0 ? a : b;
				c = neutral.getLerped(accent, ofClamp(m * 1.2f, 0, 1));

				ofSetColor(c);
				ofDrawCircle(ox + (i + 0.5f) * cell, oy + (j + 0.5f) * cell, r);
			}
		}
	}
};

std::unique_ptr<Sketch> makeSketch() { return std::make_unique<GridSketch>(); }
