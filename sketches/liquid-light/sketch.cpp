// Liquid light: a fullscreen GLSL fragment shader, seamless loop.
//
// Domain-warped value noise ("fbm of fbm") drives a smooth, flowing field, shaded with a
// cosine palette and a soft vignette. The loop trick: the shader never sees raw time, only
// a point moving on a CIRCLE (cos, sin of progress * 2pi). Everything the noise samples from
// that point is periodic with the clip duration, so the last frame flows into the first.
//
// Tunables live in params.json (no recompile): "scale", "warp", "speed", "hueShift".

#include "Sketch.h"

static const char* VERT = R"(
#version 120
void main() {
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
}
)";

static const char* FRAG = R"(
#version 120
uniform vec2  u_resolution;
uniform vec2  u_loop;       // (cos, sin) of progress * 2pi
uniform float u_scale;      // noise frequency
uniform float u_warp;       // domain warp strength
uniform float u_speed;      // how far the loop point travels through noise space
uniform float u_hueShift;   // palette offset

float hash(vec2 p) {
	p = fract(p * vec2(123.34, 456.21));
	p += dot(p, p + 45.32);
	return fract(p.x * p.y);
}

// Value noise, smooth interpolation.
float vnoise(vec2 p) {
	vec2 i = floor(p), f = fract(p);
	vec2 u = f * f * (3.0 - 2.0 * f);
	return mix(mix(hash(i), hash(i + vec2(1.0, 0.0)), u.x),
	           mix(hash(i + vec2(0.0, 1.0)), hash(i + vec2(1.0, 1.0)), u.x), u.y);
}

float fbm(vec2 p) {
	float v = 0.0, a = 0.5;
	mat2 rot = mat2(0.8, 0.6, -0.6, 0.8);
	for (int i = 0; i < 5; i++) {
		v += a * vnoise(p);
		p = rot * p * 2.0 + 10.0;
		a *= 0.5;
	}
	return v;
}

// Three-stop ramp: midnight navy -> teal -> gold. t in 0..1, wraps so hueShift can rotate it.
vec3 palette(float t) {
	t = fract(t);
	vec3 navy = vec3(0.02, 0.04, 0.16);
	vec3 teal = vec3(0.00, 0.55, 0.62);
	vec3 gold = vec3(1.00, 0.78, 0.30);
	vec3 c = mix(navy, teal, smoothstep(0.0, 0.5, t));
	c = mix(c, gold, smoothstep(0.5, 0.9, t));
	return mix(c, navy, smoothstep(0.9, 1.0, t));   // wrap back for shifted palettes
}

void main() {
	vec2 res = u_resolution;
	vec2 uv = (gl_FragCoord.xy - 0.5 * res) / min(res.x, res.y);  // centred, aspect-safe
	vec2 flow = u_loop * u_speed;                                  // periodic offset
	vec2 p = uv + vec2(37.1, 91.7) / u_scale;   // keep the hash away from the origin, where it is biased dark

	// Domain warp: q = fbm(p), r = fbm(p + q), colour from fbm(p + r)
	vec2 q = vec2(fbm(p * u_scale + flow),
	              fbm(p * u_scale + vec2(5.2, 1.3) - flow));
	vec2 r = vec2(fbm(p * u_scale + u_warp * q + vec2(1.7, 9.2) + flow.yx),
	              fbm(p * u_scale + u_warp * q + vec2(8.3, 2.8) - flow.yx));
	float f = fbm(p * u_scale + u_warp * r);

	// Shape the field. fbm lives mostly in 0.3..0.7: stretch that to 0..1 so there are
	// real darks and bright veins, and keep the mapping monotonic so nothing plateaus.
	float v = smoothstep(0.22, 0.72, f);
	float veins = smoothstep(0.50, 0.85, f + 0.3 * length(r - q));
	vec3 col = palette(v * 0.78 + 0.1 * q.x + u_hueShift);
	col *= 0.55 + 0.6 * v;                                       // lows stay deep navy, not black
	col += vec3(1.0, 0.92, 0.75) * veins * 0.25;                 // hot veins

	// Gentle vignette
	float vig = 1.0 - 0.35 * dot(uv, uv);
	col *= clamp(vig, 0.0, 1.0);

	gl_FragColor = vec4(col, 1.0);
}
)";

class LiquidLight : public Sketch {
	ofShader shader;
public:
	void setup(const Ctx& ctx) override {
		shader.setupShaderFromSource(GL_VERTEX_SHADER, VERT);
		shader.setupShaderFromSource(GL_FRAGMENT_SHADER, FRAG);
		shader.linkProgram();
	}

	void draw(const Ctx& ctx) override {
		float a = ctx.progress * TWO_PI;
		shader.begin();
		shader.setUniform2f("u_resolution", ctx.w, ctx.h);
		shader.setUniform2f("u_loop", std::cos(a), std::sin(a));
		shader.setUniform1f("u_scale",    ctx.params.value("scale", 3.2f));
		shader.setUniform1f("u_warp",     ctx.params.value("warp", 2.4f));
		shader.setUniform1f("u_speed",    ctx.params.value("speed", 0.5f));
		shader.setUniform1f("u_hueShift", ctx.params.value("hueShift", 0.0f));
		ofDrawRectangle(0, 0, ctx.w, ctx.h);
		shader.end();
	}
};

std::unique_ptr<Sketch> makeSketch() { return std::make_unique<LiquidLight>(); }
