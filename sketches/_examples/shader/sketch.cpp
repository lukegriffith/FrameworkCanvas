// Example: fullscreen fragment shader, seamless loop.
//
// Pattern to copy: the shader source is embedded as strings, so there are no file
// paths to resolve (ofShader::load() paths are relative to bin/data, which trips
// people up). To use files instead, put them in bin/data/shaders/ and call
// shader.load("shaders/name"). OF 0.12 opens a GL 2.1 context by default, so use
// "#version 120" with gl_Vertex / gl_ModelViewProjectionMatrix.
//
// Loop trick: feed the shader a CIRCLE (cos, sin of progress) instead of raw time.
// Anything driven by those two numbers is periodic with period = clip duration.

#include "Sketch.h"

static const char* VERT = R"(
#version 120
void main() {
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
	gl_TexCoord[0] = gl_MultiTexCoord0;
}
)";

static const char* FRAG = R"(
#version 120
uniform vec2  u_resolution;
uniform vec2  u_loop;       // (cos, sin) of progress * 2pi

vec3 palette(float t) {     // cosine palette, a classic
	return 0.5 + 0.5 * cos(6.28318 * (vec3(1.0) * t + vec3(0.00, 0.33, 0.67)));
}

void main() {
	vec2 p = (gl_FragCoord.xy * 2.0 - u_resolution) / min(u_resolution.x, u_resolution.y);
	vec2 q = p;
	float acc = 0.0;
	for (int i = 0; i < 5; i++) {
		q = abs(q) / dot(q, q) - 0.9 + 0.12 * u_loop;   // folded inversion, wobbled by the loop
		acc += exp(-length(q) * 1.4);
	}
	float v = acc * 0.5;
	vec3 col = palette(v + 0.15 * u_loop.x) * smoothstep(0.0, 1.0, v * 1.4);
	gl_FragColor = vec4(col, 1.0);
}
)";

class ShaderSketch : public Sketch {
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
		ofDrawRectangle(0, 0, ctx.w, ctx.h);
		shader.end();
	}
};

std::unique_ptr<Sketch> makeSketch() { return std::make_unique<ShaderSketch>(); }
