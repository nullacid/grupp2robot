import bluetooth
import sys, os, traceback

#Mac Address of our firefly module
fireflyMacAddr = '00:06:66:03:A6:96'

class Harald():
	def __init__(self):
		self.targetDevice = None
		self.port = 1
		self.ourSocket = bluetooth.BluetoothSocket(bluetooth.RFCOMM)
		self.lastCommand = b'0xff'
		
		while not self.establishDirectConnection():
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
			
	def establishDirectConnection(self):
		self.targetDevice = fireflyMacAddr
		try:
			self.ourSocket.connect((self.targetDevice, self.port))
			print("Connected to firefly module")
			return True
		except IOError:
			print("Failed to connect to firefly module, reattempting to connect")
			return False
				
	def sendData(self, data):	
		if self.targetDevice != None:
			self.ourSocket.send(data)
			self.lastCommand = data
			print("sent data: " + str(hex(data[0])))
			
	def receiveData(self):
		data = self.__waitToReceive()
		print("Data Received: " + str(hex(data[0])))
		return data
			
	def __sendConfirmation(self, data):
		if self.__checkIntegrity(data):
			self.ourSocket.send(b'\xfe')
			return True
		else:
			self.ourSocket.send(b'\xff')
			return False
		
	def __recConfirmation(self):
		data = self.__waitToReceive(1)
		if hex(data[0]) == hex(int("7f", 16)):
			return True
		return False
	
	def __waitToReceive(self):
		if self.targetDevice != None:
			while True:
				return self.ourSocket.recv(1)
			
				
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
		
		for i in range(1, len(bitArray)):
			if bitArray[i] == 1:
				sum += 1
				
		if sum % 2 == 0:
			parity = 1
		
		if parity == bitArray[0]:
			return True
			
		return False
		
			
			
			
			