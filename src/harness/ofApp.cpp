#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
	settings = Settings::fromEnv();
	totalFrames = settings.totalFrames();

	cout << "OUTPUT_TYPE: " << settings.outputType << endl;
	cout << "Video settings: " << settings.width << "x" << settings.height << " @ " << settings.fps << "fps for " << settings.duration << "s" << endl;
	cout << "seed=" << settings.seed << " renderStart=" << settings.renderStart << " renderEnd=" << settings.renderEnd << endl;

	ofSeedRandom(settings.seed);
	ofSetCircleResolution(64);   // OF default (22) shows visible facets at video resolutions

	ctx.fps = settings.fps;
	ctx.duration = settings.duration;
	ctx.seed = settings.seed;
	ctx.params = settings.params;

	if (settings.outputType == "video") {
		// Render as fast as possible; frame timing comes from the frame index, not the clock.
		// The window is only a small preview: the FBO is what is output-sized.
		ofSetVerticalSync(false);
		ofSetFrameRate(0);
		float s = std::min(1.0f, std::min(640.0f / settings.width, 640.0f / settings.height));
		ofSetWindowShape(std::max(1, (int)(settings.width * s)), std::max(1, (int)(settings.height * s)));
		ofSetWindowTitle("Recording: " + ofToString(totalFrames) + " frames");
		cout << "Video mode: recording " << totalFrames << " frames" << endl;
		beginRecording();
		renderStart = settings.renderStart;
		renderEnd = settings.renderEnd;
	} else {
		ofSetFrameRate(60);
		cout << "Interactive app mode" << endl;
	}

	// Offscreen buffer at the recording resolution, independent of window/screen size
	// (grabScreen() gets clamped by macOS to the actual window bounds, corrupting
	// tall/large frame captures).
	fbo.allocate(settings.width, settings.height, GL_RGBA, 4);   // 4x MSAA

	startTime = ofGetElapsedTimef();
	fillCtx(0);
	sketch = makeSketch();
	sketch->setup(ctx);

	ofBackground(0);
}

//--------------------------------------------------------------
void ofApp::beginRecording(){
	recording = true;
	frameCount = 0;
	renderStart = 0;
	renderEnd = totalFrames - 1;

	framesDir = settings.framesDir.empty() ? ofToDataPath("frames", true) : settings.framesDir;
	ofDirectory::createDirectory(framesDir, false, true);
}

//--------------------------------------------------------------
// Time is deterministic when recording (frame / fps) and wall clock otherwise,
// so a sketch runs at the same speed in both modes.
void ofApp::fillCtx(int frame){
	ctx.recording = recording;
	ctx.frame = frame;
	if (recording) {
		ctx.w = settings.width;
		ctx.h = settings.height;
		ctx.t = frame / (float)settings.fps;
	} else {
		ctx.w = ofGetWidth();
		ctx.h = ofGetHeight();
		ctx.t = ofGetElapsedTimef() - startTime;
		ctx.frame = ofGetFrameNum();
	}
	float p = ctx.t / ctx.duration;
	ctx.progress = p - std::floor(p);
}

//--------------------------------------------------------------
void ofApp::update(){
	if (recording && frameCount > renderEnd) {
		ofExit();
	}
}

//--------------------------------------------------------------
void ofApp::draw(){
	// ofExit() in update() doesn't stop the draw that follows it; don't capture an extra frame
	if (recording && frameCount > renderEnd) return;

	if (recording) {
		// Frames before renderStart are skipped entirely (no draw, no save)
		if (frameCount >= renderStart) {
			fillCtx(frameCount);

			fbo.begin();
			ofClear(0, 0, 0, 255);
			ofSetColor(255);
			sketch->draw(ctx);
			fbo.end();

			// Preview scaled to whatever size the window actually is
			ofSetColor(255);
			fbo.draw(0, 0, ofGetWidth(), ofGetHeight());
			ofSetColor(255, 0, 0);
			ofDrawBitmapString("Recording frame " + ofToString(frameCount + 1) + "/" + ofToString(totalFrames), 10, 20);

			saveFrame();
		}
		frameCount++;
	} else {
		fillCtx(0);
		ofClear(0, 0, 0, 255);
		ofSetColor(255);
		sketch->draw(ctx);

		ofSetColor(255);
		ofDrawBitmapString("Interactive mode - Press 'r' to start recording", 10, 20);
		ofDrawBitmapString("FPS: " + ofToString(ofGetFrameRate()), 10, 40);
	}
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
	if (key == 'r' && !recording) {
		// Start manual recording in app mode
		ofSetFrameRate(settings.fps);
		beginRecording();
		return;
	}
	sketch->keyPressed(key, ctx);
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){
	sketch->mousePressed(x, y, button, ctx);
}

//--------------------------------------------------------------
void ofApp::saveFrame(){
	ofPixels pixels;
	fbo.readToPixels(pixels);
	ofImage img;
	img.setFromPixels(pixels);

	string filename = framesDir + "/frame_" + ofToString(frameCount, 6, '0') + ".tga";
	bool saved = img.save(filename);

	if (!saved) {
		cout << "Failed to save: " << filename << endl;
	} else if (frameCount % 30 == 0) {
		cout << "Saved frame " << frameCount << endl;
	}
}
