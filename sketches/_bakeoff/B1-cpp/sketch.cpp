// B1: particle vortex. Teal and orange particles spiral inward around a dark
// centre. Stateless: each particle's life is frac(phase + k*progress) with
// integer k, and it fades to nothing at both ends of its life, so the loop is exact.

#include "Sketch.h"

class Vortex : public Sketch {
	struct P {
		float angle0, phase, size, bright, warm;
		int   k;      // lives per loop (integer)
		int   dir;    // unused sign kept for variety of speed
		float swirl;  // extra winding
	};
	std::vector<P> ps;

	static float smooth(float x) { x = ofClamp(x, 0, 1); return x * x * (3 - 2 * x); }

public:
	void setup(const Ctx&) override {
		for (int i = 0; i < 420; i++) {
			P p;
			p.angle0 = ofRandom(TWO_PI);
			p.phase  = ofRandom(1);
			p.k      = 1 + (int)ofRandom(0, 2.99f);
			p.size   = ofRandom(0.0025f, 0.0065f);
			p.bright = ofRandom(0.6f, 1.0f);
			p.warm   = ofRandom(1) < 0.45f ? 1.0f : 0.0f;
			p.swirl  = ofRandom(0.85f, 1.15f);
			ps.push_back(p);
		}
	}

	// u in any real; returns position and alpha
	bool eval(const P& p, float prog, float unit, glm::vec2& pos, float& alpha, float& r01) const {
		float u = p.phase + p.k * prog;
		u -= std::floor(u);
		float r = ofLerp(0.46f, 0.07f, std::pow(u, 0.8f));   // inward
		float a = p.angle0 + TWO_PI * (2 * prog + p.swirl * 0.35f / (r + 0.05f) * 0.5f);
		a += TWO_PI * 0;                                       // 2 whole turns per loop: seamless
		pos = { std::cos(a) * r * unit, std::sin(a) * r * unit };
		alpha = smooth(u / 0.15f) * smooth((1 - u) / 0.2f);
		r01 = u;
		return true;
	}

	void draw(const Ctx& ctx) override {
		float unit = std::min(ctx.w, ctx.h);
		ofBackground(4, 6, 10);

		ofEnableBlendMode(OF_BLENDMODE_ADD);
		ofPushMatrix();
		ofTranslate(ctx.w / 2.0f, ctx.h / 2.0f);

		// faint dark-centre halo ring
		for (int i = 0; i < 12; i++) {
			float rr = unit * (0.07f + i * 0.012f);
			ofSetColor(10, 40, 45, 6);
			ofDrawCircle(0, 0, rr);
		}

		ofColor teal(30, 215, 200), orange(255, 140, 40);
		const int tail = 28;
		const float step = 0.0011f;
		for (const P& p : ps) {
			ofColor base = p.warm > 0.5f ? orange : teal;
			for (int j = 0; j < tail; j++) {
				glm::vec2 q; float al, u;
				eval(p, ctx.progress - j * step * (float)p.k * 0.0f - j * step, unit, q, al, u);
				float f = 1.0f - j / (float)tail;
				float a = al * f * p.bright;
				if (a < 0.01f) continue;
				ofColor c = base;
				// hotter toward the centre
				c.lerp(ofColor(255, 245, 225), ofClamp((u - 0.6f) * 1.2f, 0, 0.6f));
				float s = p.size * unit * (0.5f + 0.5f * f);
				if (j == 0) {
					c.a = 28 * a; ofSetColor(c); ofDrawCircle(q.x, q.y, s * 5.0f);
					c.a = 70 * a; ofSetColor(c); ofDrawCircle(q.x, q.y, s * 2.2f);
					c.a = 255 * a; ofSetColor(c); ofDrawCircle(q.x, q.y, s);
				} else {
					c.a = 38 * a; ofSetColor(c); ofDrawCircle(q.x, q.y, s * 0.9f);
				}
			}
		}
		ofPopMatrix();
		ofDisableBlendMode();
	}
};

std::unique_ptr<Sketch> makeSketch() { return std::make_unique<Vortex>(); }
