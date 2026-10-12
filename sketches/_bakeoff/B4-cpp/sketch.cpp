// B4 glow field: 6 metaballs on closed Lissajous paths (integer frequencies => seamless loop),
// field value mapped through a soft neon ramp.
#include "Sketch.h"

static const char* VERT = R"(
#version 120
void main() { gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex; }
)";

static const char* FRAG = R"(
#version 120
uniform vec2  u_resolution;
uniform float u_a;          // progress * 2pi
const int N = 6;

vec3 ramp(float x) {
	// deep indigo -> violet -> magenta -> hot pink -> orange-ish glow -> cyan-white core
	vec3 c0 = vec3(0.02, 0.01, 0.10);
	vec3 c1 = vec3(0.30, 0.05, 0.60);
	vec3 c2 = vec3(0.95, 0.10, 0.65);
	vec3 c3 = vec3(1.00, 0.55, 0.35);
	vec3 c4 = vec3(0.30, 0.95, 1.00);
	vec3 c5 = vec3(1.00, 1.00, 1.00);
	x = clamp(x, 0.0, 1.0) * 5.0;
	if (x < 1.0) return mix(c0, c1, smoothstep(0.0, 1.0, x));
	if (x < 2.0) return mix(c1, c2, smoothstep(0.0, 1.0, x - 1.0));
	if (x < 3.0) return mix(c2, c3, smoothstep(0.0, 1.0, x - 2.0));
	if (x < 4.0) return mix(c3, c4, smoothstep(0.0, 1.0, x - 3.0));
	return mix(c4, c5, smoothstep(0.0, 1.0, x - 4.0));
}

void main() {
	float unit = min(u_resolution.x, u_resolution.y);
	vec2 p = (gl_FragCoord.xy - 0.5 * u_resolution) / unit;   // short side spans -0.5..0.5
	float aspect = u_resolution.x / unit;
	float f = 0.0;
	for (int i = 0; i < N; i++) {
		float fi = float(i);
		float ph = fi * 1.7;
		// integer frequencies 1..3 keep it periodic
		float fx = 1.0 + mod(fi, 3.0);
		float fy = 1.0 + mod(fi + 1.0, 3.0);
		vec2 c = vec2(0.42 * aspect * 0.8 * sin(fx * u_a + ph),
		              0.30 * sin(fy * u_a + ph * 1.3 + 0.7));
		c += 0.06 * vec2(cos(2.0 * u_a + fi), sin(3.0 * u_a + fi * 2.0));
		float r = 0.115 + 0.03 * sin(u_a + fi * 2.1);
		vec2 d = p - c;
		float q = dot(d, d) / (r * r);
		float w = 1.0 / (1.0 + q * q);       // smooth, long-tailed falloff
		f += w * 1.0;
	}
	float v = 1.0 - exp(-f * 1.25);          // soft saturation
	vec3 col = ramp(v);
	col += 0.10 * vec3(0.8, 0.2, 1.0) * smoothstep(0.0, 0.3, v);   // haze
	gl_FragColor = vec4(col, 1.0);
}
)";

class GlowField : public Sketch {
	ofShader shader;
public:
	void setup(const Ctx&) override {
		shader.setupShaderFromSource(GL_VERTEX_SHADER, VERT);
		shader.setupShaderFromSource(GL_FRAGMENT_SHADER, FRAG);
		shader.linkProgram();
	}
	void draw(const Ctx& ctx) override {
		shader.begin();
		shader.setUniform2f("u_resolution", ctx.w, ctx.h);
		shader.setUniform1f("u_a", ctx.progress * TWO_PI);
		ofDrawRectangle(0, 0, ctx.w, ctx.h);
		shader.end();
	}
};

std::unique_ptr<Sketch> makeSketch() { return std::make_unique<GlowField>(); }
