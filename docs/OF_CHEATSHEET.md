# openFrameworks 0.12.1 cheat sheet for sketches

Every `cpp` block here compiles inside a sketch's `draw()` (CI-style check: they are
extracted and built together). `ctx` is the `Ctx` passed to `draw`/`setup`. Blocks
tagged `text` are explanations or error messages.

## Mental model

- Origin is the top-left corner. +x right, +y down. Units are pixels.
- Draw order is paint order: later calls cover earlier ones.
- Drawing state (colour, line width, blend mode, matrix) is global and sticky.
  Set colour before every shape; wrap local changes in `ofPushStyle/ofPopStyle` and
  `ofPushMatrix/ofPopMatrix`.
- Colour channels are 0..255, including `ofColor::fromHsb(h, s, b)`.
- Size things from `ctx.w`/`ctx.h`, never from literal pixel numbers.

## Scale-independent sizing

```cpp
float unit = std::min(ctx.w, ctx.h);        // both ints: fine
float r = unit * 0.1f;                       // 10% of the short side
float cx = ctx.w / 2.0f;                     // use 2.0f: ctx.w / 2 is integer division
float pad = unit * 0.05f;
```

## Time and loops

```cpp
float a = ctx.progress * TWO_PI;             // 0..2pi across the whole clip
float pulse = std::sin(a * 3);               // integer multiple => seamless loop
float ease = 0.5f - 0.5f * std::cos(a);      // 0 -> 1 -> 0, smooth at both ends
float line = ofMap(ctx.progress, 0, 1, 0, ctx.w);   // linear sweep (wraps with a jump)
float secs = ctx.t;                          // seconds since start, for non-looping motion
```

Non-integer multiples of `ctx.progress * TWO_PI` do NOT loop. Never use
`ofGetFrameNum()`, `ofGetElapsedTimef()` or `ofGetLastFrameTime()`: they differ
between app and video mode.

## Colour and blending

```cpp
ofBackground(10, 12, 20);
ofSetColor(255, 100, 100);                   // r, g, b
ofSetColor(255, 100, 100, 128);              // with alpha
ofSetColor(ofColor::fromHsb(150, 200, 255)); // hue 0..255, sat, bright
ofColor c = ofColor::fromHsb(ofMap(ctx.progress, 0, 1, 0, 255), 200, 255);
c.a = 90;
ofSetColor(c);
ofEnableBlendMode(OF_BLENDMODE_ADD);         // glow: overlaps get brighter
ofEnableBlendMode(OF_BLENDMODE_SCREEN);
ofEnableAlphaBlending();                     // back to normal (default state)
ofDisableBlendMode();
```

Alpha blending is on by default. Additive blending on a white background does
nothing visible: use it on dark backgrounds.

## Shapes

```cpp
ofSetCircleResolution(64);                   // harness already sets 64
ofDrawCircle(ctx.w / 2.0f, ctx.h / 2.0f, 100);
ofDrawEllipse(100, 100, 200, 80);            // centre x, y, width, height
ofDrawRectangle(10, 10, 200, 100);           // top-left x, y, w, h
ofDrawRectRounded(10, 10, 200, 100, 12);
ofDrawLine(0, 0, ctx.w, ctx.h);
ofDrawTriangle(0, 0, 100, 0, 50, 80);
ofNoFill();                                   // outlines only
ofSetLineWidth(3);
ofDrawCircle(300, 300, 50);
ofFill();                                     // filled again
ofSetRectMode(OF_RECTMODE_CENTER);           // rects now drawn from their centre
ofDrawRectangle(ctx.w / 2.0f, ctx.h / 2.0f, 80, 80);
ofSetRectMode(OF_RECTMODE_CORNER);
```

## Transforms

```cpp
ofPushMatrix();
ofTranslate(ctx.w / 2.0f, ctx.h / 2.0f);     // move origin to centre
ofRotateDeg(45);                             // degrees. Prefer this to ofRotate(): the unit is explicit
ofRotateRad(ctx.progress * TWO_PI);          // or radians
ofScale(2.0f);                               // uniform; ofScale(sx, sy) also fine
ofDrawRectangle(-50, -50, 100, 100);
ofPopMatrix();                               // always pair with Push
```

## Vectors and maths

Use `glm`, not `ofVec2f`/`ofVec3f` (deprecated).

```cpp
glm::vec2 p(100.0f, 200.0f);
glm::vec2 q = p + glm::vec2(10.0f, 0.0f);
float d = glm::distance(p, q);
glm::vec2 dir = glm::normalize(q - p);
glm::vec2 m = glm::mix(p, q, 0.5f);          // lerp
float v = ofMap(0.3f, 0, 1, 10, 50, true);   // map range, last arg clamps
float l = ofLerp(0, 100, 0.25f);
float k = ofClamp(1.7f, 0, 1);
float n = ofNoise(0.3f, 0.7f);               // 0..1, smooth
float sn = ofSignedNoise(0.3f, 0.7f, 1.1f);  // -1..1
float loopN = ofNoise(1.0f, 2.0f, std::cos(ctx.progress * TWO_PI), std::sin(ctx.progress * TWO_PI));  // 4D: loops
float r = ofRandom(1.0f, 5.0f);              // seeded by SEED: reproducible
int ri = (int)ofRandom(0, 10);               // cast: ofRandom is float
```

`ofNoise` takes 1 to 4 arguments. Feeding the last two the cos/sin of
`ctx.progress * TWO_PI` makes the field animate and return exactly to its start.

## Paths and polylines

```cpp
ofPolyline line;
for (int i = 0; i <= 100; i++) {
	float t = i / 100.0f;
	line.addVertex(t * ctx.w, ctx.h / 2.0f + std::sin(t * TWO_PI * 3) * 100.0f);
}
ofSetLineWidth(2);
line.getSmoothed(3).draw();                   // smoothed copy
ofPolyline resampled = line.getResampledBySpacing(10);

ofPath path;
path.setFillColor(ofColor(255, 120, 60));
path.moveTo(100, 100);
path.lineTo(300, 100);
path.bezierTo(350, 150, 350, 250, 300, 300);
path.close();
path.draw();
```

## Meshes (many vertices, per-vertex colour)

```cpp
ofMesh mesh;
mesh.setMode(OF_PRIMITIVE_TRIANGLES);
mesh.addVertex(glm::vec3(0, 0, 0));
mesh.addColor(ofFloatColor(1, 0, 0));        // float colours are 0..1
mesh.addVertex(glm::vec3(ctx.w, 0, 0));
mesh.addColor(ofFloatColor(0, 1, 0));
mesh.addVertex(glm::vec3(ctx.w / 2.0f, ctx.h, 0));
mesh.addColor(ofFloatColor(0, 0, 1));
mesh.draw();                                  // for thousands of points: ofVboMesh, built once in setup()
```

## Text

```cpp
ofTrueTypeFont font;
font.load(OF_TTF_SANS, 64);                   // do this in setup(), once. System sans font.
font.drawString("hello", 100, 200);          // y is the BASELINE
float w = font.stringWidth("hello");
ofRectangle box = font.getStringBoundingBox("hello", 0, 0);
```

Fonts from a file: put the `.ttf` in `bin/data/` and `load("MyFont.ttf", 64)`; paths
are relative to `bin/data`, not the working directory. `ofDrawBitmapString` is a
fixed 8px debug font and does not scale: do not use it in output.

## Shaders

```cpp
ofShader shader;
shader.setupShaderFromSource(GL_VERTEX_SHADER, "#version 120\nvoid main(){gl_Position=gl_ModelViewProjectionMatrix*gl_Vertex;}");
shader.setupShaderFromSource(GL_FRAGMENT_SHADER, "#version 120\nuniform float u_t;\nvoid main(){gl_FragColor=vec4(vec3(u_t),1.0);}");
shader.linkProgram();                          // all of the above belongs in setup()
shader.begin();
shader.setUniform1f("u_t", ctx.progress);
ofDrawRectangle(0, 0, ctx.w, ctx.h);
shader.end();
shader.load("shaders/my");                    // loads bin/data/shaders/my.vert + my.frag
```

The context is OpenGL 2.1: use `#version 120`, `gl_Vertex`, `gl_FragColor`,
`gl_FragCoord`. GLSL 150+ syntax (`in`/`out`, `layout`) fails to compile, the output is
black, and stdout shows `ERROR: 0:1: '' :  version '150' is not supported`. See `sketches/_examples/shader`.

## Images

```cpp
ofImage img;
img.load("photo.jpg");                        // relative to bin/data. In setup(), not draw()
img.draw(0, 0, ctx.w, ctx.h);
ofSetColor(255);                              // images are tinted by the current colour
```

## Feedback and trails with an extra FBO

```cpp
ofFbo trail;
trail.allocate(ctx.w, ctx.h, GL_RGBA);        // in setup()
trail.begin();
ofSetColor(0, 0, 0, 12);                      // fade previous content a little
ofDrawRectangle(0, 0, ctx.w, ctx.h);
ofSetColor(255);
ofDrawCircle(ctx.w * ctx.progress, ctx.h / 2.0f, 20);
trail.end();
ofSetColor(255);
trail.draw(0, 0);
```

Anything that accumulates between frames (feedback, simulation, trails stored in an
FBO) depends on every earlier frame having been drawn. `make frame T=...` skips the
earlier frames, so its output will not match the same moment in a full render. Use
`make sheet` or `make draft` to judge such sketches, and prefer stateless
closed-form motion (see `sketches/_examples/particles`) when you can.

## Common compile and runtime errors

```text
error: no matching function for call to 'min'   (or 'max')
```
Mixed int/float arguments, e.g. `std::min(ctx.w, 0.5f)`. Make both the same type:
`std::min((float)ctx.w, 0.5f)`, or use `ofClamp`.

Degrees vs radians: `ofRotate(x)` takes degrees but reads like radians, and `sin`/`cos`
take radians. A wrong unit compiles fine and just looks wrong. Prefer `ofRotateDeg` and
`ofRotateRad`.

```text
Undefined symbols for architecture arm64: "makeSketch()"
```
The sketch file does not define `std::unique_ptr<Sketch> makeSketch()`, or defines it
with a different signature, or `SKETCH=` points at a directory without `sketch.cpp`.

```text
error: cannot initialize a parameter of type 'const Ctx &' ...  /  'override' but does not override
```
Method signatures must match `src/Sketch.h` exactly: `void draw(const Ctx& ctx) override`.

Output is all black, no error: an empty `draw()`, a shader that failed to compile
(check stdout for `ofShader` errors), colour left at alpha 0, everything drawn
off-screen because of a unit mix-up, or `ofNoFill()` left on. `make smoke` catches
the fully blank case.

Shapes look wrong after a few frames: you changed global state and did not undo it.
Pair `ofPushMatrix/ofPopMatrix` and `ofPushStyle/ofPopStyle`.

Integer division: `ctx.w / 2` is an int. Use `2.0f`.

`ofDrawCircle` inside a huge loop is slow with high circle resolution. For many tiny
dots use `ofDrawRectangle`, a mesh with `OF_PRIMITIVE_POINTS`, or lower
`ofSetCircleResolution` (and set it back).
