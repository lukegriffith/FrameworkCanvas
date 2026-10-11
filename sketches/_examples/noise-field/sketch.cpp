// Example: flow-field streamlines through looping noise.
//
// Pattern to copy: ofNoise takes up to 4 inputs. Walking a circle in the last two
// (cos/sin of progress) makes the field animate AND return exactly to its start,
// so the loop is seamless with no cross-fade. Streamlines are re-traced every frame,
// so nothing is stored between frames.

#include "Sketch.h"

class NoiseField : public Sketch {
	std::vector<glm::vec2> seeds;   // start points as fractions of the canvas (0..1), fixed by SEED
	std::vector<float> hues;

	// Angle of the flow at (x,y) in unit space. ofNoise returns 0..1.
	float flowAngle(float x, float y, float progress) const {
		const float loopR = 0.6f;                       // bigger = more change over the loop
		float a = progress * TWO_PI;
		float n = ofNoise(x * 1.6f, y * 1.6f, std::cos(a) * loopR, std::sin(a) * loopR);
		return n * TWO_PI * 2.0f;
	}

public:
	void setup(const Ctx& ctx) override {
		for (int i = 0; i < 2200; i++) {
			seeds.push_back({ ofRandom(-0.1f, 1.1f), ofRandom(-0.1f, 1.1f) });
			hues.push_back(ofRandom(0, 1));
		}
	}

	void draw(const Ctx& ctx) override {
		ofBackground(244, 238, 226);                      // paper
		// Work in "unit space": 1.0 = the short side. Lengths and noise frequency then look
		// the same in square, portrait and wide formats, and seeds still cover the whole canvas.
		float unit = std::min(ctx.w, ctx.h);

		ofSetLineWidth(std::max(1.0f, unit / 700.0f));
		const int steps = 40;
		const float stepLen = 0.006f;

		for (size_t i = 0; i < seeds.size(); i++) {
			ofPolyline line;
			glm::vec2 p(seeds[i].x * ctx.w / unit, seeds[i].y * ctx.h / unit);
			for (int s = 0; s < steps; s++) {
				line.addVertex(p.x * unit, p.y * unit);
				float a = flowAngle(p.x, p.y, ctx.progress);
				p += glm::vec2(std::cos(a), std::sin(a)) * stepLen;
			}
			// Two inks: deep blue and warm red, chosen per line.
			ofColor c = hues[i] < 0.8f ? ofColor(24, 38, 82, 150) : ofColor(200, 60, 40, 190);
			ofSetColor(c);
			line.draw();
		}
	}
};

std::unique_ptr<Sketch> makeSketch() { return std::make_unique<NoiseField>(); }
