#
#  init.py
#  BlenderKinect
#
#  Created by João Claro.
#  Copyright (c) 2014 FCT/UNL. All rights reserved.
#
import bge
import socket
import pickle
import sys
import time
from threading import Thread

HOST = 'localhost'#socket.gethostbyname(socket.gethostname())
PORT = 10124 # Arbitrary non-privileged port
global sock
print('[INIT]')
print('Socket at:',(HOST, PORT))

try:
	sock
	print('Socket is defined?')
except NameError:
	sock = None
	print('Socket is defined as None')

# Datagram (udp) socket
if sock is None:
	try :
		sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
		sock.setblocking(0)
		print ('Socket created')
	except (socket.error, msg):
		print ('Failed to create socket. Error Code : ' + str(msg[0]) + ' Message ' + str(msg[1]))
		sys.exit()
		 
	# Bind socket to local host and port
	try:
		sock.bind((HOST, PORT))
		print ('Socket bind complete')
	except socket.error as sockErr:
		print ('Bind failed.', sockErr)
		sys.exit()
		