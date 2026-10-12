// B3: layered dusk ridgelines drifting with parallax. Each ridge is periodic in x with
// period P_i; over the clip it shifts by exactly one period, so the loop is seamless.
#include "Sketch.h"

namespace {
const int NL = 9;
struct Layer {
	float period;   // in units of screen width
	float baseY;    // fraction of height
	float amp;      // fraction of height
	float phase[4];
	int   harm[4];
	ofColor col;
	float dir;
};
}

class Ridges : public Sketch {
public:
	Layer L[NL];

	void setup(const Ctx&) override {
		ofColor back(255, 150, 90), mid(176, 82, 120), front(18, 30, 70);
		for (int i = 0; i < NL; i++) {
			float k = i / float(NL - 1);
			Layer& l = L[i];
			l.period = 0.35f + 1.9f * k;          // front layers move faster
			l.baseY = 0.30f + 0.62f * std::pow(k, 0.9f);
			l.amp = 0.05f + 0.05f * k;
			// two-stage gradient: warm -> magenta -> deep blue
			ofColor c = k < 0.5f ? back.getLerped(mid, k * 2) : mid.getLerped(front, (k - 0.5f) * 2);
			l.col = c;
			l.dir = -1;
			// harmonic counts scale with period so feature size stays similar on screen
			float s = l.period * (1.6f + 0.5f * (1 - k));
			int base = std::max(2, int(std::round(s)));
			l.harm[0] = base; l.harm[1] = base * 2 + 1; l.harm[2] = base * 3 + 1; l.harm[3] = base * 5 + 2;
			for (int j = 0; j < 4; j++) l.phase[j] = ofRandom(TWO_PI);
		}
	}

	static float ridge(const Layer& l, float u) {   // u in periods
		float a[4] = {1.0f, 0.5f, 0.28f, 0.12f};
		float v = 0;
		for (int j = 0; j < 4; j++) v += a[j] * std::sin(TWO_PI * l.harm[j] * u + l.phase[j]);
		return v / 1.4f;   // roughly -1..1
	}

	void draw(const Ctx& ctx) override {
		float W = ctx.w, H = ctx.h;
		// sky gradient
		ofMesh sky;
		sky.setMode(OF_PRIMITIVE_TRIANGLE_STRIP);
		ofColor top(40, 40, 100), hor(255, 190, 120);
		sky.addVertex({0, 0, 0}); sky.addColor(top);
		sky.addVertex({W, 0, 0}); sky.addColor(top);
		sky.addVertex({0, H * 0.45f, 0}); sky.addColor(ofColor(150, 80, 130));
		sky.addVertex({W, H * 0.45f, 0}); sky.addColor(ofColor(150, 80, 130));
		sky.addVertex({0, H * 0.7f, 0}); sky.addColor(hor);
		sky.addVertex({W, H * 0.7f, 0}); sky.addColor(hor);
		sky.draw();

		// sun glow
		ofSetColor(255, 220, 150, 90);
		ofDrawCircle(W * 0.5f, H * 0.36f, W * 0.2f);
		ofSetColor(255, 235, 190, 200);
		ofDrawCircle(W * 0.5f, H * 0.36f, W * 0.09f);

		const int step = 6;
		for (int i = 0; i < NL; i++) {
			const Layer& l = L[i];
			float P = l.period * W;
			float shift = l.dir * ctx.progress * P;   // exactly one period per loop
			ofMesh m;
			m.setMode(OF_PRIMITIVE_TRIANGLE_STRIP);
			// fade toward haze at the bottom of each layer
			ofColor bot = l.col.getLerped(ofColor(255, 170, 130), 0.0f);
			ofColor topc = l.col;
			ofColor lower = l.col.getLerped(i + 1 < NL ? L[i + 1].col : l.col, 0.6f);
			for (float x = 0; x <= W + step; x += step) {
				float u = (x - shift) / P;
				float y = (l.baseY + l.amp * ridge(l, u)) * H;
				m.addVertex({x, y, 0}); m.addColor(topc);
				m.addVertex({x, H, 0}); m.addColor(lower);
			}
			m.draw();
		}
	}
};

std::unique_ptr<Sketch> makeSketch() { return std::make_unique<Ridges>(); }
