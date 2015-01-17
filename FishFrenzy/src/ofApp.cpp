#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){

	ofSetVerticalSync(true);			// Set graphics stuff
	ofSetLogLevel(OF_LOG_NOTICE);		// Debug level

	kinect.setRegistration(true);		// Registrate depth checking

	kinect.init(false, false);			// Disables video image (faster fps)
	kinect.open();						// Opens first available kinect

	kinectAngle = 0;					// Level kinect at startup
	kinect.setCameraTiltAngle(kinectAngle);

	// Debug kinect startup messages
	if (kinect.isConnected()) 
	{
		ofLogNotice() << "sensor-emitter dist: " << kinect.getSensorEmitterDistance() << "cm";
		ofLogNotice() << "sensor-camera dist:  " << kinect.getSensorCameraDistance() << "cm";
		ofLogNotice() << "zero plane pixel size: " << kinect.getZeroPlanePixelSize() << "mm";
		ofLogNotice() << "zero plane dist: " << kinect.getZeroPlaneDistance() << "mm";
	}

	// Populate and init image with correct image size
	grayImage.allocate(kinect.width, kinect.height);

	useThreshold = true;				// Tell openCV to use threshold values
	nearThreshold = 255;				// Check as near as possible
	farThreshold = 243;					// Optimal for fist tracking

	// Set box2d parameters
	box2d.init();
	box2d.setGravity(0, 0);				// Set a vector2 for the gravity
	box2d.createBounds();				// Create the bounds of the box2d world
	box2d.setFPS(30.0);					// Set the physics update rate

	// Add two 'hands'
	ofPtr<ofxBox2dRect> box = ofPtr<ofxBox2dRect>(new ofxBox2dRect);
	box->setPhysics(3.0, 0.53, 0.1);
	box->setup(box2d.getWorld(), ofGetWidth() * 0.75f, ofGetHeight() * 0.5f, 120, 50);
	box->setFixedRotation(true);
	box->setRotationFriction(1000);
	box->setRotation(0);
	boxes.push_back(box);
	
	for (int i = 0; i < 15; i++)
	{
		boxes.push_back(ofPtr<ofxBox2dRect>(new ofxBox2dRect));
		boxes.back().get()->setPhysics(3.0, 0.53, 0.1);
		boxes.back().get()->setup(box2d.getWorld(), ofRandomWidth(), 50, 50, 50);
	}

	ofSetFrameRate(60);					// Try to keep the display framerate at 60fps
}

//--------------------------------------------------------------
void ofApp::update(){

	kinect.update();					// Update kinect first to get images

	if (kinect.isFrameNew())			// Update code only when kinect has new data
	{
		// Load greyscale depth images
		grayImage.setFromPixels(kinect.getDepthPixels(), kinect.width, kinect.height);
		grayImage.resize(ofGetWidth(), ofGetHeight());	// Set the image so that it covers the whole screen
		grayImage.mirror(false, true);	// For some reason the image is mirrored :S so we mirror it back!
		
		// Do treshold!
		unsigned char *pixels = grayImage.getPixels();
		int numPixels = grayImage.getWidth() * grayImage.getHeight();
		for (int i = 0; i < numPixels; i++)
		{
			// Change the pixels to white and black so we can see them better later
			pixels[i] = (pixels[i] < nearThreshold && pixels[i] > farThreshold) ? 255 : 0;
		}

		grayImage.flagImageChanged();	// Tells open CV that the image was updated

		// Find contours
		contourFinder.findContours(grayImage, 30, (kinect.width * kinect.height) * 0.5f, 20, false); 
	}

	// Now we have the blobs from findContours, iterate trough them
	if (contourFinder.nBlobs > 0 && boxes.size() > 0) {
		ofxCvBlob blob = contourFinder.blobs.at(0);
		ofxBox2dRect *box = boxes.front().get();

		ofVec2f blobPos = ofVec2f(blob.centroid.x, blob.centroid.y);
		float distance = box->getPosition().distance(blobPos);
		ofVec2f force = (blobPos - box->getPosition()).normalized();
		box->setVelocity(force * 0.1f * distance);

		box->setRotation(0);
	}

	// Update movement of boxes
	for (int i=0; i<boxes.size(); i++) 
	{
		ofxBox2dRect *box = boxes[i].get();
		if (box->getPosition().y < ofGetHeight() * 0.5f)
		{
			box->addForce(ofVec2f(0, 30), 1.0f);
		}
		else
		{
			box->friction = 10.0f;
		}
	}

	// After everything is done
	box2d.update();
}

//--------------------------------------------------------------
void ofApp::draw(){

	ofSetColor(150);										// Set default background color a nice shade of gray

	grayImage.draw(0, 0, grayImage.width, grayImage.height);		// Draw threshold image
	contourFinder.draw(0, 0, grayImage.width, grayImage.height);	// Draw the found contours

	// Draw box2D
	for (int i=0; i<boxes.size(); i++) {
		ofFill();
		ofSetHexColor(0xBF2545);
		boxes[i].get()->draw();
	}

	// Draw water
	ofSetColor(0, 0, 255, 150); 
	ofRect(0, ofGetHeight() * 0.5f, ofGetWidth(), ofGetHeight() * 0.5f);

	// draw the ground
	box2d.drawGround();
}

//--------------------------------------------------------------
void ofApp::exit() {

	kinect.setCameraTiltAngle(0); // zero the tilt on exit
	kinect.close();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
	
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
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}