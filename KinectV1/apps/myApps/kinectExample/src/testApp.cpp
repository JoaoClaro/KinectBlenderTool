#include <iostream>
#include "testApp.h"

#define TAB 9
#define HIPS 0
#define X 120
#define V 118
#define D 100

#define SCALE 40

//--------------------------------------------------------------
void testApp::setup(){

	ofSetWindowTitle("Kinect Capturing Tool");

	kinect.initSensor();
	//kinect.initIRStream(640, 480);
	kinect.initColorStream(640, 480, true);
	kinect.initDepthStream(640, 480, true);
	kinect.initSkeletonStream(false);

	//simple start
	kinect.start();
	ofDisableAlphaBlending(); //Kinect alpha channel is default 0;
	ofSetWindowShape(640, 480);

	//initialize sockets
	sockAddress = Poco::Net::SocketAddress("localhost", 10124);
	//socket.connect(sockAddress);
	
	loadXML("../../src/kinect_tpose.xml");

	depth = false;
	debug = false;
	video = "";
}

//--------------------------------------------------------------
void testApp::update(){
	kinect.update();
}

//--------------------------------------------------------------
void testApp::draw()
{
	if(ofGetWidth() != 640 || ofGetHeight() != 480)
		ofSetWindowShape(640, 480);

	if(depth)	
		kinect.drawDepth(0, 0);
	else		
		kinect.draw(0,0);

	ofPushStyle();
	ofSetColor(0, 0, 0);
	ofSetLineWidth(3.0f);

	skeletons = kinect.getSkeletons();
	ss.str("");

	// process all skeletal data
	for(int i=0; i<skeletons.size(); i++){
		skeleton = skeletons.at(i);

		if(skeleton.size() > 0){
			for(auto & bone : skeleton) {
				int bIndex = bone.first;
				for(int dIndex=0; dIndex < allJoints[bIndex].size(); dIndex++){ // for all recorded destinations from current joint

					startrot = bone.second.getRotationMatrix().getRotate();
					if(startrot == ofQuaternion(0,0,0,0)) // NULL node verification
						startrot = ofQuaternion(0,0,0,1);

					allJoints[bIndex][dIndex].pos = bone.second.getStartPosition();
					allJoints[bIndex][dIndex].rot = startrot;

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
					
					// Calculate floor
					if(bone.first == HIPS){
						ground = (bone.second.ground.x * allJoints[bIndex][dIndex].pos.x + bone.second.ground.y * allJoints[bIndex][dIndex].pos.y + bone.second.ground.z * allJoints[bIndex][dIndex].pos.z + bone.second.ground.w);
					}

					if(bone.first != 15 && bone.first != 19){
					ss << setprecision(3) << fixed << video << "|" << allJoints[bIndex][dIndex].destination << "|" 
					<< (-bone.second.position.x*SCALE) << "|" << (bone.second.position.z*SCALE-SCALE) << "|" << (ground*SCALE) << "|"
					<< allJoints[bIndex][dIndex].rot.w() << "|" << allJoints[bIndex][dIndex].rot.x() << "|" 
					<< allJoints[bIndex][dIndex].rot.y() << "|" << allJoints[bIndex][dIndex].rot.z() << ";";
					}
					/* //DEBUG PRINT
					if(bone.first == 0){
						ofLog(OF_LOG_NOTICE, "%d (%f, %f, %f, %f)", bone.first, allJoints[bIndex][dIndex].rot.w(),allJoints[bIndex][dIndex].rot.x(), allJoints[bIndex][dIndex].rot.y(),allJoints[bIndex][dIndex].rot.z());
					}
					*/

					//draw bone connections
					auto index = bone.second.getStartJoint();
					auto connectedTo = skeleton.find((_NUI_SKELETON_POSITION_INDEX) index);
					if (connectedTo != skeleton.end()) {
						ofSetColor(255, 0, 255);
						ofLine(connectedTo->second.getScreenPosition(), bone.second.getScreenPosition());
					}

					switch(bone.second.getTrackingState()) {
						case SkeletonBone::Inferred:
							ofSetColor(0, 0, 255);
							break;
						case SkeletonBone::Tracked:
							ofSetColor(0, 255, 0);
							break;
						case SkeletonBone::NotTracked:
							ofSetColor(255, 0, 0);
							break;
					}
					ofSetColor(255, 255, 255); //TODO: remove
					ofCircle(bone.second.getScreenPosition(), 10.0f);
					ofSetColor(0, 0, 0);
					ofDrawBitmapString(to_string(bone.first), floor(bone.second.getScreenPosition().x)-4, floor(bone.second.getScreenPosition().y)+4);

					if(debug){
						//draw joint direction axis
						float angle = 2*acos(startrot.w());
						direction = ofVec3f(startrot.x(), startrot.y(), startrot.z());
						startpos = ofVec3f(allJoints[bIndex][dIndex].pos.x, allJoints[bIndex][dIndex].pos.y, 0);
						x_vec = ofVec3f(30, 0, 0);
						y_vec = ofVec3f(0, 30, 0);
						z_vec = ofVec3f(0, 0, 30);

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
						ofSetColor(0,0,255);
						drawPillar( startpos.x, startpos.y, startpos.z, startpos.x+z_vec.x, startpos.y-z_vec.y, startpos.z-z_vec.z);
					}
				}
			}
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
void testApp::loadXML(string path){

	invCheck.clear();
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

			invCheck[id][j].clear();
			if( skeletonxml.exists(invert) )
				invCheck[id][j] = skeletonxml.getValue(invert);
			else
				invCheck[id][j] == "";

			ofQuaternion result(ofToFloat(skeletonxml.getValue(getx)), ofToFloat(skeletonxml.getValue(gety)), 
									ofToFloat(skeletonxml.getValue(getz)), ofToFloat(skeletonxml.getValue(getw)));
			allJoints[id].back().rotations.push_back( result );

		}
		skeletonxml.setToParent();
	}
	
	ofLog(OF_LOG_NOTICE, "File "+path.substr(path.find_last_of("/\\")+1,path.length())+" loaded succesfully." );
	
}

//-------------------------------------------------------------
void testApp::drawPillar(float x1, float y1, float z1, float x2, float y2, float z2){
	ofLine( x1, y1, z1, x2, y2, z2);
	ofLine( x1-1, y1, z1, x2-1, y2, z2);
	ofLine( x1+1, y1, z1, x2+1, y2, z2);
	ofLine( x1, y1-1, z1, x2, y2-1, z2);
	ofLine( x1, y1+1, z1, x2, y2+1, z2);
	ofLine( x1, y1, z1-1, x2, y2, z2-1);
	ofLine( x1, y1, z1+1, x2, y2, z2+1);
}

//--------------------------------------------------------------
void testApp::keyPressed(int key){

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

	if(key == D)
		debug = !debug;

	//ofLog(OF_LOG_NOTICE, "%d", key );
}

//--------------------------------------------------------------
void testApp::keyReleased(int key){

}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y){

}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void testApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void testApp::dragEvent(ofDragInfo dragInfo){ 

}