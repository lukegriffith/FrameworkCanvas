#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
	loadVideoSettings();
	
	cout << "OUTPUT_TYPE: " << outputType << endl;
	cout << "Video settings: " << videoWidth << "x" << videoHeight << " @ " << videoFPS << "fps for " << videoDuration << "s" << endl;
	
	if (outputType == "video") {
		ofSetWindowShape(videoWidth, videoHeight);
		ofSetFrameRate(videoFPS);
		isRecording = true;
		frameCount = 0;
		totalFrames = videoDuration * videoFPS;
		ofSetWindowTitle("Recording: " + ofToString(totalFrames) + " frames");
		cout << "Video mode: recording " << totalFrames << " frames" << endl;
		
		// Create frames directory
		ofDirectory::createDirectory(ofToDataPath("frames"), false, false);
	} else {
		isRecording = false;
		ofSetFrameRate(60);
		cout << "Interactive app mode" << endl;
	}
	
	// Add some basic visual content for demo
	ofBackground(0);
}

//--------------------------------------------------------------
void ofApp::update(){
	if (isRecording && frameCount >= totalFrames) {
		ofExit();
	}
}

//--------------------------------------------------------------
void ofApp::draw(){
	// Demo animation - rotating colorful circle
	ofPushMatrix();
	ofTranslate(ofGetWidth()/2, ofGetHeight()/2);
	ofRotateDeg(ofGetFrameNum() * 2);
	
	ofSetColor(255, 100, 100);
	ofDrawCircle(0, 0, 100 + 50 * sin(ofGetFrameNum() * 0.1));
	
	ofSetColor(100, 255, 100);
	ofDrawCircle(150, 0, 30);
	
	ofSetColor(100, 100, 255);
	ofDrawCircle(-150, 0, 30);
	ofPopMatrix();
	
	// Show recording status
	if (isRecording) {
		ofSetColor(255, 0, 0);
		ofDrawBitmapString("Recording frame " + ofToString(frameCount+1) + "/" + ofToString(totalFrames), 10, 20);
		saveFrame();
		frameCount++;
	} else {
		ofSetColor(255);
		ofDrawBitmapString("Interactive mode - Press 'r' to start recording", 10, 20);
		ofDrawBitmapString("FPS: " + ofToString(ofGetFrameRate()), 10, 40);
	}
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
	if (key == 'r' && !isRecording) {
		// Start manual recording in app mode
		isRecording = true;
		frameCount = 0;
		totalFrames = videoDuration * videoFPS;
		ofSetFrameRate(videoFPS);
		ofDirectory::createDirectory(ofToDataPath("frames"), false, false);
	}
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}

//--------------------------------------------------------------
void ofApp::loadVideoSettings(){
	// Load settings from environment variables (set by Makefile)
	char* envOutputType = getenv("OUTPUT_TYPE");
	outputType = envOutputType ? string(envOutputType) : "app";
	
	char* envWidth = getenv("VIDEO_WIDTH");
	videoWidth = envWidth ? ofToInt(envWidth) : 1920;
	
	char* envHeight = getenv("VIDEO_HEIGHT");
	videoHeight = envHeight ? ofToInt(envHeight) : 1080;
	
	char* envFPS = getenv("VIDEO_FPS");
	videoFPS = envFPS ? ofToInt(envFPS) : 30;
	
	char* envDuration = getenv("VIDEO_DURATION");
	videoDuration = envDuration ? ofToInt(envDuration) : 10;
}

//--------------------------------------------------------------
void ofApp::saveFrame(){
	ofImage img;
	img.grabScreen(0, 0, ofGetWidth(), ofGetHeight());
	
	string filename = ofToDataPath("frames/frame_" + ofToString(frameCount, 6, '0') + ".png");
	bool saved = img.save(filename);
	
	if (saved) {
		cout << "Saved: " << filename << endl;
	} else {
		cout << "Failed to save: " << filename << endl;
	}
}