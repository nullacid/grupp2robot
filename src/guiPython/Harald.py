import bluetooth
import sys, os, traceback
import struct

from time import *

#Mac Address of our firefly module
fireflyMacAddr = '00:06:66:03:A6:96'


#Should socket maybe be closed?
class Harald():
	def __init__(self):
		self.targetDevice = None
		self.port = 1
		self.ourSocket = bluetooth.BluetoothSocket(bluetooth.RFCOMM)
		self.lastCommand = b'0xff'
		
		while not self.establishConnection():
			pass
		
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
				
	def sendData(self, data):	
		print("send Data")
		if self.targetDevice != None:
			self.ourSocket.send(data)
			self.lastCommand = data
			print("sent data: " + str(hex(data[0])))
			if self.__recConfirmation():
				print("Data Error, resending command")
				#self.sendData(data)
			
	def receiveData(self, numBytes):
		print("receive data")
		tja = True
		while tja:
			data = self.__waitToReceive(numBytes)
			if self.__sendConfirmation(data):
				tja = False
		if self.__checkIntegrity(data):
			return data
		else:
			print("Integrity error, resending request")
			self.sendData(self.lastCommand)
			
	def __sendConfirmation(self, data):
		print("send confirmation")
		#sleep(5)
		if self.__checkIntegrity(data):
			print("good data")
			self.ourSocket.send(b'\x7e')
			return True
		else:
			self.ourSocket.send(b'\x7f')
			return False
		
	def __recConfirmation(self):
		print("receive confirmation")
		data = self.__waitToReceive(1)
		print("data: " + hex(data[0]))
		if hex(data[0]) == hex(int("7f", 16)):
			print("tja")
			return True
		return False
	
	def __waitToReceive(self, numBytes):
		if self.targetDevice != None:
			while True:
				return self.ourSocket.recv(numBytes)
			
				
	def __checkIntegrity(self, data):
		bitArray = []
		for b in self.__convertToBits(data):
			bitArray.insert(0, b)
			
		return self.__checkParity(bitArray)
	def __convertToBits(self, data):
		for b in data:
			for i in range(8):
				yield(b >> i) & 1
				
	def __checkParity(self, bitArray):
		sum = 0
		parity = 0
		
		print(bitArray[0], end = "")
		for i in range(1, len(bitArray)):
			print(bitArray[i], end = "")
			if bitArray[i] == 1:
				sum += 1
		print("\n")
				
		if sum % 2 == 0:
			parity = 1
		
		if parity == bitArray[0]:
			print("Parity good")
			return True
			
		return False
		
			
			
			
			