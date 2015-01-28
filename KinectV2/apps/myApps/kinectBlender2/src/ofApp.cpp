//
//  ofApp.cpp
//  KinectBlender2
//
//  Created by João Claro.
//  Copyright (c) 2014 FCT/UNL. All rights reserved.
//
#include <iostream>
#include "ofApp.h"

#define TAB 9
#define HIPS 0
#define X 120
#define V 118
#define SPACE 32
#define TRIM -180

#define SCALE 40

//--------------------------------------------------------------
void ofApp::setup(){

	ofSetWindowTitle("KinectV2 Capturing Tool");

	//initialize kinect
	kinect.initSensor();
	kinect.initColorStream(false, KCBColorImageFormat_Rgba);
	//kinect.initIRStream();
	kinect.initDepthStream(true);
	//kinect.initBodyIndexStream();
	kinect.initSkeletonStream(false);

	kinect.start();
	ofDisableAlphaBlending(); //Kinect alpha channel is default 0;
	ofSetVerticalSync(true);

	//initialize sockets
	sockAddress = Poco::Net::SocketAddress("localhost", 10124);
	//socket.connect(sockAddress);

	loadXML("../../src/kinectV2_tpose.xml");

	depth = true;
	video = "";

	bAlignment = false;
}

//--------------------------------------------------------------
void ofApp::update(){
	kinect.update();
}

//--------------------------------------------------------------
void ofApp::draw()
{
	if(ofGetWidth() != 652 || ofGetHeight() != 540)
		ofSetWindowShape(652, 540);

	ofBackground(0, 255);

	if(depth)
		kinect.drawDepth(0, 0, 652, 540);
	else
		kinect.draw(TRIM, 0, 960, 540);

	ofPushStyle();
	ofSetColor(0, 0, 0);
	ofSetLineWidth(3.0f);

	skeletons = kinect.getSkeletons();	
	joints.clear();
	ss.str("");

	// get all skeleton joints
	for(auto & skeleton : skeletons) { // for all skeleton joints
		if(skeleton.tracked){ // if found skeleton
			joints = skeleton.joints;
		}
	}

	// process all skeletal data
	for(auto & bone : joints) { // for all skeleton joints
		int bIndex = bone.first;
		for(int dIndex=0; dIndex < allJoints[bIndex].size(); dIndex++){ // for all recorded destinations from current joint

			rot = bone.second.getOrientation(); //TODO: replace rot after debugging	
			if(rot == ofQuaternion(0,0,0,0)) // NULL node verification
				rot = ofQuaternion(0,0,0,1);

			originals[bone.first] = rot;

			allJoints[bIndex][dIndex].pos = bone.second.getScreenPosition();
			allJoints[bIndex][dIndex].rot = rot;

			switch(bone.first){
				case 0: local[0] = originals[0];
				case 1: local[1] = originals[0].conj()*originals[1];break;
				case 2: local[2] = originals[20].conj()*originals[2]; break;
				case 3: local[3] = originals[2].conj()*originals[3]; break;
				case 4: local[4] = originals[20].conj()*originals[4]; break;
				case 5: local[5] = originals[4].conj()*originals[5]; break;
				case 6: local[6] = originals[5].conj()*originals[6]; break;
				case 7: local[7] = originals[6].conj()*originals[7]; break;
				case 8: local[8] = originals[20].conj()*originals[8]; break;
				case 9: local[9] = originals[8].conj()*originals[9]; break;
				case 10: local[10] = originals[9].conj()*originals[10]; break;
				case 11: local[11] = originals[10].conj()*originals[11]; break;
				case 12: local[12] = originals[0].conj()*originals[12]; break;
				case 13: local[13] = originals[12].conj()*originals[13]; break;
				case 14: local[14] = originals[13].conj()*originals[14]; break;
				case 15: local[15] = originals[14].conj()*originals[15]; break;
				case 16: local[16] = originals[0].conj()*originals[16]; break;
				case 17: local[17] = originals[16].conj()*originals[17]; break;
				case 18: local[18] = originals[17].conj()*originals[18]; break;
				case 19: local[19] = originals[18].conj()*originals[19]; break;
				case 20: local[20] = originals[1].conj()*originals[20]; break;
				case 21: local[21] = originals[7].conj()*originals[21]; break;
				case 22: local[22] = originals[21].conj()*originals[22]; break;
				case 23: local[23] = originals[11].conj()*originals[23]; break;
				case 24: local[24] = originals[23].conj()*originals[24]; break;
			}

			// Dependency check
			if( nodeParent[bIndex].first != bIndex ){
				for(int i=0; i < allJoints[nodeParent[bIndex].first].size(); i++){
					if(nodeParent[bIndex].second == allJoints[nodeParent[bIndex].first][i].destination)
						allJoints[bIndex][dIndex].rot *= allJoints[nodeParent[bIndex].first][i].rot;
				}	
			}
			
			// Additional World alignments
			if( !allJoints[bIndex][dIndex].alignments.empty() ){
				for(int i=0; i < allJoints[bIndex][dIndex].alignments.size(); i++){
					allJoints[bIndex][dIndex].rot = allJoints[bIndex][dIndex].alignments[i]*allJoints[bIndex][dIndex].rot;
				}
			}
			

			
			// Additional Local rotations
			if( !allJoints[bIndex][dIndex].rotations.empty() ){
				for(int i=0; i < allJoints[bIndex][dIndex].rotations.size(); i++){
					allJoints[bIndex][dIndex].rot *= allJoints[bIndex][dIndex].rotations[i].conj();					
					
				}
			}
			
			
			// Nullify Additional World alignments
			if( !allJoints[bIndex][dIndex].alignments.empty() ){
				for(int i=allJoints[bIndex][dIndex].alignments.size()-1; i >=0 ; i--){
					allJoints[bIndex][dIndex].rot = allJoints[bIndex][dIndex].rot*allJoints[bIndex][dIndex].alignments[i].inverse();
				}
			}
			
			//TODO: better ground recognition
			ground = 0.9;
			//if(bIndex == HIPS)
				//ground = (skeleton.ground.x * bone.second.getPosition().x + skeleton.ground.y * bone.second.getPosition().y + skeleton.ground.z * bone.second.getPosition().z + skeleton.ground.w);
			//ofLog(OF_LOG_NOTICE, "%d (%f)", bIndex, ground);

			
			

			// Arm information has too much noise
			//if( bIndex != 4 || bIndex != 8 ){
				ss << setprecision(3) << fixed << video << "|" << allJoints[bIndex][dIndex].destination << "|" 
				<< (-bone.second.getPosition().x*SCALE) << "|" << (bone.second.getPosition().z*SCALE-SCALE) << "|" << (ground*SCALE) << "|"
				<< allJoints[bIndex][dIndex].rot.w() << "|" << allJoints[bIndex][dIndex].rot.x() << "|" 
				<< allJoints[bIndex][dIndex].rot.y() << "|" << allJoints[bIndex][dIndex].rot.z() << ";";
			//}

			// draw joints and connections
			drawIndex = kinect.getSkeletonDrawOrder(bIndex).first;
			ofSetColor(255, 255, 255);

			ofPoint parentpos(allJoints[drawIndex][0].pos.x+TRIM, allJoints[drawIndex][0].pos.y);
			ofPoint childpos(allJoints[bIndex][dIndex].pos.x+TRIM, allJoints[bIndex][dIndex].pos.y);
			ofLine( parentpos, childpos );

			switch(bone.second.getTrackingState()) {
				case TrackingState_Inferred:
					ofSetColor(0, 0, 255);
					break;
				case TrackingState_Tracked:
					ofSetColor(0, 127, 0);
					break;
				case TrackingState_NotTracked:
					ofSetColor(255, 0, 0);
					break;
			}
			if(bAlignment)
				ofSetColor(255, 255, 0);
			ofCircle( childpos, 10.0f );
			ofSetColor(0, 0, 0);
			ofDrawBitmapString(to_string(bIndex), floor(allJoints[bIndex][dIndex].pos.x)-4+TRIM, floor(allJoints[bIndex][dIndex].pos.y)+4);

			if(bAlignment){ // show vectors after alignment
				if( !allJoints[bIndex][dIndex].alignments.empty() ){
					for(int i=0; i < allJoints[bIndex][dIndex].alignments.size(); i++){
						rot = allJoints[bIndex][dIndex].alignments[i]*rot;
					}
				}
			}

			/*
			//draw joint direction axis || debug mode?????
			float angle = 2*acos(rot.w());
			direction = ofVec3f(rot.x(), rot.y(), rot.z());
			startpos = ofVec3f(allJoints[bIndex][dIndex].pos.x+TRIM, allJoints[bIndex][dIndex].pos.y, 0);
			x_vec = ofVec3f(60, 0, 0);
			y_vec = ofVec3f(0, 60, 0);
			z_vec = ofVec3f(0, 0, 60);

			x_vec.rotateRad(angle, direction); 
			y_vec.rotateRad(angle, direction); 
			z_vec.rotateRad(angle, direction); 
			
			// draw X vector direction
			ofSetColor(255,0,0);
			drawPillar( startpos.x, startpos.y, startpos.z, startpos.x+x_vec.x, startpos.y-x_vec.y, startpos.z-x_vec.z );
			// draw Y vector direction
			ofSetColor(0,255,0);
			drawPillar( startpos.x, startpos.y, startpos.z, startpos.x+y_vec.x, startpos.y-y_vec.y, startpos.z-y_vec.z);
			// draw Z vector direction
			if(startpos.z-z_vec.z < 0)	ofSetColor(192,0,255);	// backward
			else						ofSetColor(0,0,255);	// forward
			drawPillar( startpos.x, startpos.y, startpos.z, startpos.x+z_vec.x, startpos.y-z_vec.y, startpos.z-z_vec.z);
			*/
		}
	}

	// Send all joints information
	if(!ss.str().empty()){
		try{
			socket.connect(sockAddress);
			msg = ss.str().substr(0,ss.str().length()-1);
			socket.sendBytes(msg.data(), msg.size());
			//ofLog(OF_LOG_NOTICE, msg);
		}
		catch (Poco::Exception& exc){
			cerr << exc.displayText() << endl;
		}
	}

	ofSetColor(255, 255, 255);
	ofDrawBitmapString("Press X to choose XML armature file.", 10, ofGetHeight()-25);
	ofDrawBitmapString("Press V to choose AVI video file.", 10, ofGetHeight()-10);
	ofPopStyle();
}

//--------------------------------------------------------------
void ofApp::loadXML(string path){

	nodeParent.clear();
	allJoints.clear();
	
	ofFile file;
	file.open(path);
	ofBuffer buffer = file.readToBuffer();
	skeletonxml.loadFromBuffer(buffer.getText());

	skeletonxml.setTo("/nodes");
	for(int i=0; i < skeletonxml.getNumChildren(); i++){ // search all <node> children for dependencies

		string nodepath = "/node["+ofToString(i)+"]";
		string getid = nodepath+"[@id]";
		string parent = nodepath+"[@parent]";
		string bone = nodepath+"[@bone]";

		if(skeletonxml.exists(parent))
			nodeParent[skeletonxml.getIntValue(getid)] = pair<int,string>(skeletonxml.getIntValue(parent), skeletonxml.getValue(bone));
		else
			nodeParent[skeletonxml.getIntValue(getid)] = pair<int,string>(skeletonxml.getIntValue(getid), "");

	}
	skeletonxml.setToParent();
	
	skeletonxml.setTo("/alignments");
	for(int i=0; i < skeletonxml.getNumChildren(); i++){ // search all <bone> children for alignments

		string alignpath = "/align["+ofToString(i)+"]";
		string getid = alignpath+"[@id]";
		string getname = alignpath+"[@name]";
		int id = ofToInt(skeletonxml.getValue(getid));
		string name = skeletonxml.getValue(getname);

		if(allJoints[id].empty()){
			allJoints[id] = vector<Bone>();
			allJoints[id].push_back( Bone(id, name) );
		}

		skeletonxml.setToChild(i);
		for(int j=0; j < skeletonxml.getNumChildren(); j++){ // for all rotations in current <bone>
			
			string rotpath = "/rotation["+ofToString(j)+"]";
			string getw = rotpath+"[@w]";
			string getx = rotpath+"[@x]";
			string gety = rotpath+"[@y]";
			string getz = rotpath+"[@z]";

			ofQuaternion result(ofToFloat(skeletonxml.getValue(getx)), ofToFloat(skeletonxml.getValue(gety)), 
									ofToFloat(skeletonxml.getValue(getz)), ofToFloat(skeletonxml.getValue(getw)));
			allJoints[id].back().alignments.push_back( result );
		}
		skeletonxml.setToParent();
	}
	skeletonxml.setToParent();

	skeletonxml.setTo("/rotations");
	for(int i=0; i < skeletonxml.getNumChildren(); i++){ // search all <bone> children for rotations

		string bonepath = "/bone["+ofToString(i)+"]";
		string getid = bonepath+"[@id]";
		string getname = bonepath+"[@name]";
		int id = ofToInt(skeletonxml.getValue(getid));
		string name = skeletonxml.getValue(getname);

		if(allJoints[id].empty()){
			allJoints[id] = vector<Bone>();
			allJoints[id].push_back( Bone(id, name) );
		}

		skeletonxml.setToChild(i);
		for(int j=0; j < skeletonxml.getNumChildren(); j++){ // for all rotations in current <bone>
			
			string rotpath = "/rotation["+ofToString(j)+"]";
			string invert = rotpath+"[@invert]";
			
			string getw = rotpath+"[@w]";
			string getx = rotpath+"[@x]";
			string gety = rotpath+"[@y]";
			string getz = rotpath+"[@z]";

			ofQuaternion result(ofToFloat(skeletonxml.getValue(getx)), ofToFloat(skeletonxml.getValue(gety)), 
									ofToFloat(skeletonxml.getValue(getz)), ofToFloat(skeletonxml.getValue(getw)));
			allJoints[id].back().rotations.push_back( result );

		}
		skeletonxml.setToParent();
	}
	
	ofLog(OF_LOG_NOTICE, "File "+path.substr(path.find_last_of("/\\")+1,path.length())+" loaded succesfully." );
	
}

//-------------------------------------------------------------
void ofApp::drawPillar(float x1, float y1, float z1, float x2, float y2, float z2){
	ofLine( x1, y1, z1, x2, y2, z2);
	ofLine( x1-1, y1, z1, x2-1, y2, z2);
	ofLine( x1+1, y1, z1, x2+1, y2, z2);
	ofLine( x1, y1-1, z1, x2, y2-1, z2);
	ofLine( x1, y1+1, z1, x2, y2+1, z2);
	ofLine( x1, y1, z1-1, x2, y2, z2-1);
	ofLine( x1, y1, z1+1, x2, y2, z2+1);
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

	//Depth to Color
	if(key == TAB){
		depth = !depth;
	}
	//Load XML
	if(key == X){
		ofFileDialogResult fileDialogResult;
		bool isXML = false;
		while(!isXML){
			fileDialogResult = ofSystemLoadDialog("Choose XML armature file", false, "./../../kinect.xml");	
			if(fileDialogResult.getName().substr(fileDialogResult.getName().length()-4,fileDialogResult.getName().length()) == ".xml")
				isXML = !isXML;
		}
		loadXML(fileDialogResult.getPath());
	}
	//Load video
	if(key == V){
		//open video
		ofFileDialogResult fileDialogResult;
		bool isVideo = false;
		while(!isVideo){
			fileDialogResult = ofSystemLoadDialog("Choose video (avi) file", false, "./../../video.avi");	
			if(fileDialogResult.getName().substr(fileDialogResult.getName().length()-4,fileDialogResult.getName().length()) == ".avi")
				isVideo = !isVideo;
		}
		video = fileDialogResult.getPath();
	}
	//Activate alignment vectors
	if(key == SPACE)
		bAlignment = !bAlignment;

	//ofLog(OF_LOG_NOTICE, "%d", key );
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
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}