import bluetooth
import sys, os, traceback
import select
from coolthread import *
from time import *


#Mac Address of our firefly module
fireflyMacAddr = '00:06:66:03:A6:96'

class Harald():
	def __init__(self):
		self.targetDevice = None
		self.port = 1
		self.ourSocket = bluetooth.BluetoothSocket(bluetooth.RFCOMM)

		while not self.establishDirectConnection():
			pass
		
	#Finds all nearby bluetooth devices and tries to connect to our firefly if it finds it
	#Not used anymore.
	def establishConnection(self):
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
		self.targetDevice = fireflyMacAddr
		try:
			self.ourSocket.connect((self.targetDevice, self.port))
			print("Connected to firefly module")
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
		receiverThread = coolThread(1, "cool receiver mega thread", self.__waitToReceive)
		receiverThread.start()
		
		timeStamp = time()
		while not receiverThread.done:
			if time() > timeStamp + 1:
				print("timed out")
				return b'\x00'
		data = receiverThread.returnValue
		#print("Data Received: " + str(hex(data[0])))
		return data
			
	def __waitToReceive(self):
		if self.targetDevice != None:
			return self.ourSocket.recv(1)
			

		
			
			
			
			