#
#  quit.py
#  BlenderKinect
#
#  Created by João Claro.
#  Copyright (c) 2014 FCT/UNL. All rights reserved.
#
import bge
import init
from threading import Thread

def quit(sock):
	cont = bge.logic.getCurrentController()
	sen = cont.sensors["Escape"]
	if sen.key == 130:
		sock.close()
		bge.logic.endGame()

thread = Thread(target = quit, args=[init.sock])
thread.start()