//
//  main.cpp
//  KinectBlender2
//
//  Created by João Claro.
//  Copyright (c) 2014 FCT/UNL. All rights reserved.
//
#include "ofMain.h"
#include "ofApp.h"

#include "ofGLProgrammableRenderer.h"

//========================================================================
int main( ){

	ofPtr<ofBaseRenderer> renderer(new ofGLProgrammableRenderer(true));
	ofSetCurrentRenderer(renderer);

	ofSetupOpenGL(1920,1080, OF_WINDOW);			// <-------- setup the GL context

	// this kicks off the running of my app
	// can be OF_WINDOW or OF_FULLSCREEN
	// pass in width and height too:
	ofRunApp( new ofApp() );

}
