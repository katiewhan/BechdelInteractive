#include "ofApp.h"

//#define DEBUG_MODE

//--------------------------------------------------------------
void ofApp::setup() {
	int numVideos = 4;
	for (int i = 0; i < numVideos; i++) {
		ofPtr<ofxHapPlayer> p(new ofxHapPlayer());
		p->load(ofToString(i) + ".mov");
		p->play();
		videos.push_back(p);

		vector<ofPtr<ofxHapPlayer>> missV;
		ofPtr<ofxHapPlayer> mp(new ofxHapPlayer());
		mp->load(ofToString(i) + "M.mov");
		mp->play();
		missV.push_back(mp);

		ofPtr<ofxHapPlayer> msp(new ofxHapPlayer());
		msp->load(ofToString(i) + "MS.mov");
		msp->play();
		missV.push_back(msp);

		//mp->setLoopState(OF_LOOP_NONE);

		missVideos.push_back(missV);

		// 0: first miss video, 1: second miss video, 2: original video
		showMissVideo.push_back(2);
	}

	kinect.setRegistration(true);
	kinect.init();
	kinect.open();
	camAngle = -90;
	kinect.setCameraTiltAngle(camAngle);
	result.allocate(kinect.getWidth(), kinect.getHeight());
	nearThreshResult.allocate(kinect.getWidth(), kinect.getHeight());
	farThreshResult.allocate(kinect.getWidth(), kinect.getHeight());
	background.allocate(kinect.getWidth(), kinect.getHeight());
	backgroundDiff.allocate(kinect.getWidth(), kinect.getHeight());   

	captureBg = true;
	isConfigured = false;

	// Adjustable parameters
	float kinectArea = kinect.width * kinect.height;
	nearThreshold.set("Near Threshold", 70, 0, 255);
	farThreshold.set("Far Threshold", 220, 0, 255);
	minArea.set("Min Area", 10, 0, kinectArea);
	maxArea.set("Max Area", kinectArea / 2, 0, kinectArea);
	movementThreshold.set("Movement Threshold", 1, 0, 3);
	useBackgroundSubtraction.set("Use Background Subtraction", false);

	configureButton.addListener(this, &ofApp::setConfiguration);

	gui.setup("Blob Tracking");
	gui.add(nearThreshold);
	gui.add(farThreshold);
	gui.add(minArea);
	gui.add(maxArea);
	gui.add(movementThreshold);
	gui.add(useBackgroundSubtraction);
	gui.add(configureButton.setup("Configure"));
}

//--------------------------------------------------------------
void ofApp::update(){
	for (int i = 0; i < 4; i++) {
		videos[i]->update();
		missVideos[i][0]->update();
		missVideos[i][1]->update();

		//if (!missVideos[i]->isPlaying() && showMissVideo[i]) {
			//showMissVideo[i] = false;
			//videos[i]->setPosition(0);
		//}
	}

	kinect.update();
	if (kinect.isFrameNew()) {
		result.setFromPixels(kinect.getDepthPixels());
		if (captureBg) {
			background = result;
			captureBg = false;
		}

		// background subtraction
		if (useBackgroundSubtraction) {
			backgroundDiff.absDiff(background, result);
			nearThreshResult = backgroundDiff;
			farThreshResult = backgroundDiff;
		}
		else {
			nearThreshResult = result;
			farThreshResult = result;
		}

		// depth thresholding
		nearThreshResult.threshold(nearThreshold);
		farThreshResult.threshold(farThreshold, true);  
		cvAnd(nearThreshResult.getCvImage(), farThreshResult.getCvImage(), result.getCvImage(), NULL);

		// find blobs
		contourFinder.findContours(result, minArea, maxArea, 10, false);
	}
}

//--------------------------------------------------------------
void ofApp::draw() {
	ofSetBackgroundColor(0);

	//int y = 800;

	// blob detection
	#ifndef DEBUG_MODE
	vector<int> foundIndices;
	for (int b = 0; b < contourFinder.blobs.size(); b++) {
		//y += 15;
		ofPoint centroid = contourFinder.blobs[b].centroid;
		int index = floor(centroid.x / (kinect.width / 4));
		foundIndices.push_back(index);
		//showMissVideo[index] = true;
		//if (!missVideos[index]->isPlaying()) {
			//missVideos[index]->setPosition(videos[index]->getPosition());
			//missVideos[index]->play();
		//}
		cout << ofToString(centroid) << " " << index << endl;
		//ofDrawBitmapString(centroid, 10, y);
		//ofDrawBitmapString(index, 200, y);
	}
	for (int j = 0; j < showMissVideo.size(); j++) {
		vector<int>::iterator it = find(foundIndices.begin(), foundIndices.end(), j);
		if (it != foundIndices.end() && showMissVideo[j] == 2) {
			showMissVideo[j] = round(ofRandom(1));
		}
		else if (it == foundIndices.end()) {
			showMissVideo[j] = 2;
		}
	}
	#endif

	if (!isConfigured) {
		ofDrawBitmapString(contourFinder.nBlobs, 10, 800);

		// configuration mode - show kinect images
		kinect.drawDepth(10, 10, 400, 300);
		kinect.draw(420, 10, 400, 300);
		backgroundDiff.draw(830, 10, 400, 300);

		nearThreshResult.draw(10, 320, 400, 300);
		farThreshResult.draw(420, 320, 400, 300);
		result.draw(830, 320, 400, 300);

		contourFinder.draw(420, 630, 400, 300);
		gui.draw();
	}
	else {
		// installation mode - show movie footage
		int x = 0;
		int w = ofGetWidth() / 4;
		for (int i = 0; i < videos.size(); i++) {
			int h = (w / videos[i]->getWidth()) * videos[i]->getHeight();
			int y = (ofGetHeight() / 2) - (h / 2);
			if (showMissVideo[i] < 2) {
				missVideos[i][showMissVideo[i]]->draw(x, y, w, h);
			}
			else {
				videos[i]->draw(x, y, w, h);
			}
			x += w;
		}
	}
}

void ofApp::exit() {
	configureButton.removeListener(this, &ofApp::setConfiguration);
	kinect.close();
}

void ofApp::setConfiguration() {
	isConfigured = true;
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key) {
	if (!isConfigured) {
		if (key == OF_KEY_RETURN) {
			captureBg = true;
		}
		else if (key == OF_KEY_DOWN) {
			camAngle--;
			kinect.setCameraTiltAngle(camAngle);
		}
		else if (key == OF_KEY_UP) {
			camAngle++;
			kinect.setCameraTiltAngle(camAngle);
		}
	}

	#ifdef DEBUG_MODE
	if (key > 47 && key < 52) {
		vector<int> foundIndices;
		foundIndices.push_back(key - 48);
		for (int j = 0; j < showMissVideo.size(); j++) {
			vector<int>::iterator it = find(foundIndices.begin(), foundIndices.end(), j);
			if (it != foundIndices.end() && showMissVideo[j] == 2) {
				showMissVideo[j] = round(ofRandom(1));
			}
			else if (it == foundIndices.end()) {
				showMissVideo[j] = 2;
			}
		}
		//showMissVideo[idx] = !showMissVideo[idx];
	}
	#endif
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

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
