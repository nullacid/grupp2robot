import sys, os, traceback
import select

import bluetooth
from time import *


#Mac Address of our firefly module
fireflyMacAddr = '00:06:66:03:A6:96'

class Harald():
	def __init__(self):
		self.targetDevice = fireflyMacAddr
		self.port = 1
		self.ourSocket = None
		
		self.connectionstatus = 0

		while not self.establishDirectConnection():
			sleep(1)
			pass
		
		
	#Finds all nearby bluetooth devices and tries to connect to our firefly if it finds it
	#Not used anymore.
	def establishConnection(self):
		self.ourSocket = bluetooth.BluetoothSocket(bluetooth.RFCOMM)
		print("performing inquiry...")
		nearby_devices = []
		try:
			nearby_devices = bluetooth.discover_devices(
			duration=8, lookup_names=True, flush_cache=True, lookup_class=False)
		except IOError:
			print("Can't find any bluetooth devices")

		print("found %d devices" % len(nearby_devices))
		for addr, name in nearby_devices:
			try:
				print("  %s - %s" % (addr, name))
			except UnicodeEncodeError:
				print("  %s - %s" % (addr, name.encode('utf-8', 'replace')))
				
		for i in range(0, len(nearby_devices)):
			if nearby_devices[i][0] == fireflyMacAddr:
				self.targetDevice = nearby_devices[i][0]
				
		if self.targetDevice != None:
			try:
				self.ourSocket.connect((self.targetDevice, self.port))
				print("Connected to firefly module")
				return True
			except IOError:
				print("Firefly module timed out, reattempting to connect")
				return False
		else:
			print("Failed to connect to firefly module, reattemping to connect")
			return False
		
	#Establishes a connection to our firefly module without looking for it first.
	#Returns true if it connects, false otherwise.
	def establishDirectConnection(self):
		
		#If socket is open from before, close it NOT SURE IF THIS IS NEEDED
		if self.ourSocket != None:
			self.ourSocket.close()
		#Open the socket
		self.ourSocket = bluetooth.BluetoothSocket(bluetooth.RFCOMM)

		#Disable timeout to give enough time for connection
		self.ourSocket.settimeout(None)

		try:
			self.ourSocket.connect((self.targetDevice, self.port))
			print("Connected to firefly module")
			
			#Clear any data from the old bluetooth connection
			self.ourSocket.settimeout(2.0)
			try:
				synctrash = self.ourSocket.recv(1)
				synctrash2 = self.ourSocket.recv(1)
			except bluetooth.BluetoothError:
				print("No data on bluetooth")

			return True
		except IOError:
			print("Failed to connect to firefly module, reattempting to connect")
			return False
				
	#Sends one byte of data to the bluetooth module
	def sendData(self, data):	
		if self.targetDevice != None:
			self.ourSocket.send(data)
			#print("sent data: " + str(hex(data[0])))
			
	def receiveData(self):
		if self.targetDevice != None:
			data = self.__attemptReceive()
			if not data:
				while not self.establishDirectConnection():
					sleep(1)
				self.receiveData()
			else:
				#print("Received data: " + str(hex(data[0])))
				return data
				

	def __attemptReceive(self):
		data = None
		self.ourSocket.settimeout(2.0)
		try:
			data = self.ourSocket.recv(1)
			return data
		except bluetooth.BluetoothError:
			print("timeout yo")
			return False

	def inc_status(self):
		if self.connectionstatus < 3:
			self.connectionstatus += 1
		else:
			self.connectionstatus = 0
		
			
			
			
			
