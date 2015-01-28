//
//  ofApp.h
//  KinectBlender2
//
//  Created by João Claro.
//  Copyright (c) 2014 FCT/UNL. All rights reserved.
//
#pragma once

#include "ofMain.h"
#include "ofxKinectCommonBridge.h"
#include "Poco/Net/DatagramSocket.h"
#include "Poco/Net/SocketAddress.h"
#include "Poco/Exception.h"

struct Bone {
	int id;
	string destination;
	vector<ofQuaternion> rotations;
	vector<ofQuaternion> alignments;
	ofQuaternion rot;
	ofVec3f pos;

	Bone(int init_id, string init_destination){
		id = init_id;
		destination = init_destination;
		rotations.clear();
		alignments.clear();
		rot.set(0,0,0,1);
		pos.set(0,0,0);
	}
} ;

class ofApp : public ofBaseApp{
	public:
		void setup();
		void update();
		void draw();
		void loadXML(string path);
		void drawPillar(float x1, float y1, float z1, float x2, float y2, float z2);
		
		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y);
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);

		ofxKinectCommonBridge kinect;

		vector<Kv2Skeleton> skeletons;
		Kv2Skeleton skeleton;
		ofXml skeletonxml;
		JointType drawIndex;
		string msg;
		ofVec3f direction, startpos, x_vec, y_vec, z_vec;

		map<int,pair<int,string>> nodeParent;	// <kinectIndex, <index,name>>
		map<int,vector<Bone>> allJoints;		// <kinectIndex, vector<Bone>>

		map<JointType, Kv2Joint> joints;
		map<int, ofQuaternion> originals;
		map<int, ofQuaternion> local;
		bool bAlignment; //Debug boolean
		ofQuaternion rot;

		Poco::Net::SocketAddress sockAddress;
		Poco::Net::DatagramSocket socket;
		stringstream ss;

		bool depth;
		double ground;
		string video;
};
