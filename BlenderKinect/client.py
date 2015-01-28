#
#  client.py
#  BlenderKinect
#
#  Created by João Claro.
#  Copyright (c) 2014 FCT/UNL. All rights reserved.
#
import socket, sys, pickle

# create dgram udp socket
try:
	s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
except socket.error:
	print ('Failed to create socket')
	sys.exit()

host = 'localhost'#socket.gethostbyname(socket.gethostname())
port = 10124

print('Trying to connect')

while(1) :
	msg = input('Enter message to send : ')
	
	try :
		#Set the whole string
		s.sendto(bytes(msg,"utf8"), (host, port))

		# receive data from client (data, addr)
		d = s.recvfrom(2048)
		reply = d[0]
		addr = d[1]

		print ('Server reply : ' + str(reply))

	except socket.error as msg:
		if msg.errno == 10054:
			print ("[ERROR] Server is unreachable")
		continue
		sys.exit()