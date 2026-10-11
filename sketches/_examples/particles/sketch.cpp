// Example: seeded particle swarm, seamless loop, stateless.
//
// Pattern to copy: every particle's position is a CLOSED-FORM function of
// ctx.progress, not an integrated simulation. That makes the loop perfect
// (progress 1.0 == progress 0.0), makes any frame renderable on its own
// (make frame T=...), and makes trails free: just evaluate earlier progress.

#include "Sketch.h"

class Particles : public Sketch {
	struct P {
		float angle0;   // starting angle
		float radius;   // base orbit radius, in units of the short side
		int   spin;     // whole turns per loop (integer => seamless)
		float wobble;   // radius wobble amplitude
		int   wobbleK;  // wobble cycles per loop (integer)
		float phase;
		float hue;
		float size;
	};
	std::vector<P> ps;

	glm::vec2 pos(const P& p, float prog, float unit) const {
		float a = p.angle0 + p.spin * TWO_PI * prog;
		float r = p.radius * (1.0f + p.wobble * std::sin(TWO_PI * (p.wobbleK * prog + p.phase)));
		return { std::cos(a) * r * unit, std::sin(a) * r * unit };
	}

public:
	void setup(const Ctx& ctx) override {
		// ofRandom is seeded by the harness (SEED env var), so this is reproducible.
		for (int i = 0; i < 500; i++) {
			P p;
			p.angle0  = ofRandom(TWO_PI);
			p.radius  = ofRandom(0.08f, 0.48f);
			p.spin    = (ofRandom(1) < 0.5f ? 1 : -1) * (int)ofRandom(1, 4);
			p.wobble  = ofRandom(0.05f, 0.25f);
			p.wobbleK = (int)ofRandom(1, 4);
			p.phase   = ofRandom(1);
			p.hue     = ofMap(p.radius, 0.08f, 0.48f, 120, 230);   // teal -> blue
			p.size    = ofRandom(0.002f, 0.006f);
			ps.push_back(p);
		}
	}

	void draw(const Ctx& ctx) override {
		float unit = std::min(ctx.w, ctx.h);

		ofBackground(8, 10, 20);
		ofEnableBlendMode(OF_BLENDMODE_ADD);   // overlapping glow
		ofPushMatrix();
		ofTranslate(ctx.w / 2.0f, ctx.h / 2.0f);

		const int tail = 10;
		const float tailStep = 0.004f;          // progress units between tail samples
		for (const P& p : ps) {
			for (int j = 0; j < tail; j++) {
				float prog = ctx.progress - j * tailStep;   // may go negative: fine, it's all sin/cos
				float fade = 1.0f - j / (float)tail;
				ofSetColor(ofColor::fromHsb(p.hue, 200, 255, 90 * fade));
				glm::vec2 q = pos(p, prog, unit);
				ofDrawCircle(q.x, q.y, p.size * unit * (0.4f + 0.6f * fade));
			}
		}
		ofPopMatrix();
		ofDisableBlendMode();
	}
};

std::unique_ptr<Sketch> makeSketch() { return std::make_unique<Particles>(); }
