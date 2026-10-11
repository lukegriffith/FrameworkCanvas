#pragma once

// HARNESS: not for sketch authors. Write sketches in sketches/<name>/sketch.cpp
// against src/Sketch.h instead.

#include "ofMain.h"
#include "Settings.h"
#include "Sketch.h"

class ofApp : public ofBaseApp {
	public:
		void setup();
		void update();
		void draw();

		void keyPressed(int key);
		void mousePressed(int x, int y, int button);

	private:
		Settings settings;
		Ctx ctx;
		std::unique_ptr<Sketch> sketch;
		ofFbo fbo;                 // output-sized render target (recording only)

		bool recording = false;
		int frameCount = 0;        // frames elapsed while recording
		int totalFrames = 0;
		int renderStart = 0, renderEnd = 0;
		float startTime = 0;
		std::string framesDir;

		void beginRecording();
		void fillCtx(int frame);
		void saveFrame();
};
