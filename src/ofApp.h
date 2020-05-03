#pragma once

#include "ofMain.h"
#include "ofxOpenCv.h"
#include "ofxKinect.h"
#include "ofxGui.h"
#include "ofxHapPlayer.h"

class ofApp : public ofBaseApp{

	public:
		int numVideos;
		vector<ofPtr<ofxHapPlayer>> videos;
		vector<vector<ofPtr<ofxHapPlayer>>> missVideos;
		vector<int> showMissVideo;

		ofxKinect kinect;
		ofxCvGrayscaleImage background;
		ofxCvGrayscaleImage backgroundDiff;
		bool captureBg;
		bool isConfigured;
		int camAngle;

		ofxCvGrayscaleImage result;
		ofxCvGrayscaleImage nearThreshResult;
		ofxCvGrayscaleImage farThreshResult;
		ofxCvContourFinder contourFinder;
		
		ofxPanel gui;
		ofParameter<int> nearThreshold;
		ofParameter<int> farThreshold;
		ofParameter<float> minArea;
		ofParameter<float> maxArea;
		ofParameter<float> movementThreshold;
		ofParameter<bool> useBackgroundSubtraction;
		ofxButton configureButton;

		void setup();
		void update();
		void draw();
		void exit();

		void setConfiguration();

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void mouseEntered(int x, int y);
		void mouseExited(int x, int y);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
		
};
