#include "ofMain.h"
#include "Poco/Net/DatagramSocket.h"
#include "Poco/Net/SocketAddress.h"
#include "Poco/Exception.h"

#define SCALE 40

class MsgThread : public ofThread{

	public:
		map<int,pair<string,string>> bones;

		//--------------------------
		void setBone(int b, string bone, ofVec3f pos, ofQuaternion r, double ground, string *video){
			ofVec3f p(pos.x, pos.y, pos.z);
			stringstream ss;
			ss.precision(3);
			ss << setprecision(3) << fixed << *video << "|" << bone << "|" << (-p.x*SCALE) << "|" << (p.z*SCALE-SCALE) << "|" << (ground*SCALE) << "|"
				<< r.w() << "|" << r.x() << "|" << r.y() << "|" << r.z() << ";";
			string str = ss.str();

			bones[b].first = bone;
			bones[b].second = str.data();
			if(*video != "") *video = "";
		}

		//--------------------------
		MsgThread(){

		}

		void start(){
            startThread(false, true);   // blocking, verbose
        }

        void stop(){
            stopThread();
        }

		//--------------------------
		void threadedFunction(){
			if(bones.size() != 0){
				try{
					Poco::Net::SocketAddress sa("localhost", 10124);
					Poco::Net::DatagramSocket dgs;
					dgs.connect(sa);

					stringstream ss;
					ss.precision(3);
					map<int,pair<string,string>>::iterator it;
					for (it=bones.begin(); it!=bones.end(); ++it){
						ss << it->second.second;
						bones[it->first].second = ""; // clean bone info so it doesn't send
					}

					if(ss.str().length() > 0){
						string msg = ss.str().substr(0,ss.str().length()-1);
						dgs.sendBytes(msg.data(), msg.size());
						//ofLog(OF_LOG_NOTICE, msg);
					}
					ss.flush();

				}
				catch (Poco::Exception& exc){
					cerr << exc.displayText() << endl;
				}
			}
		}

		

};
