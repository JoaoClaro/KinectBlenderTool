#
#  server.py
#  BlenderKinect
#
#  Created by João Claro.
#  Copyright (c) 2014 FCT/UNL. All rights reserved.
#
import init
import bge
import math
import mathutils
import socket
import pickle
import sys
import time
import signal
import sys
import Rasterizer
import threading
import VideoTexture
from threading import Thread

Rasterizer.showMouse(1);
scene = bge.logic.getCurrentScene()
rig = bge.logic.getCurrentScene().objects.get('rig')
controller = bge.logic.getCurrentController()
obj = controller.owner

if("Video" in obj):
	video = obj["Video"]
	video.refresh(True)
else:
	matID = VideoTexture.materialID(obj, "MA" + obj['material'])
	video = VideoTexture.Texture(obj, matID)
	movieName = obj['movie']
	movie = bge.logic.expandPath('//' + movieName)
	video.source = VideoTexture.VideoFFmpeg("")
	video.source.scale = True
	obj["Video"] = video
	video.source.repeat = -1
	video.source.play()
	

def loop(sock):

	try:
		
		d = sock.recvfrom(2048)
		data = d[0].decode("utf-8")
		addr = d[1]	
		
		list = str(data).split(';')

		movie = list[0].split('|')[0]
		if(movie != obj['movie'] and movie != "" and movie != None and movie[0] != '\x00'):
			video.source = VideoTexture.VideoFFmpeg(movie)
			video.source.scale = True
			obj["Video"] = video
			video.source.repeat = -1
			video.source.play()
			obj['movie'] = movie
			
		for item in list:
			if(len(item.split('|')) >= 9):
				bone = item.split('|')
				bone_name = bone[1]		
				
				if bone_name in rig.channels:
				
					rig.channels[bone_name].rotation_mode = 0
					
					if bone_name == 'hips':
						rig.channels['torso'].location = ( (float(bone[2])/2), (float(bone[3])/2), (float(bone[4])/2)-20 )
					
					if bone_name == 'forearm.fk.L' or bone_name == 'forearm.fk.R':
						if (float(bone[5])*float(bone[6]) > 0):
							rig.channels[bone_name].rotation_quaternion = (float(bone[5]), float(bone[6]), 0, 0)
						else:
							rig.channels[bone_name].rotation_quaternion = (1, 0, 0, 0)
					elif bone_name == 'shin.fk.L' or bone_name == 'shin.fk.R':
						if (float(bone[5])*float(bone[6]) < 0):
							rig.channels[bone_name].rotation_quaternion = (float(bone[5]), float(bone[6]), 0, 0)
						else:
							rig.channels[bone_name].rotation_quaternion = (1, 0, 0, 0)
					else:
						rig.channels[bone_name].rotation_quaternion = (float(bone[5]), float(bone[6]), float(bone[7]), float(bone[8]))
				
		rig.update()
		
	except socket.error as sockErr:
		#print ('Recv failed:', sockErr)
		pass

			
thread = Thread(target = loop, args=[init.sock])
thread.start()
