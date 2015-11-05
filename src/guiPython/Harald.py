import bluetooth
import sys, os, traceback

#Mac Address of our firefly module
fireflyMacAddr = '00:06:66:03:A6:96'


#Should socket maybe be closed?
class Harald():
	def __init__(self):
		self.targetDevice = None
		self.port = 1
		self.ourSocket = bluetooth.BluetoothSocket(bluetooth.RFCOMM)
		
		#while not self.establishConnection():
		#	pass
		self.establishConnection()
		
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
			self.ourSocket.connect((self.targetDevice, self.port))
			print("Connected to firefly module")
			return True
		else:
			print("Failed to connect to firefly module, reattemping to connect")
			return False
				
	def sendData(self, data):
		#Should probably make sure that data is good before sending
		if self.targetDevice != None:
			self.ourSocket.send(data)
			print("sent data " + data)
			
	def waitToReceive(self, numBytes):
		if self.targetDevice != None:
			while True:
				return hex(self.ourSocket.recv(numBytes)[0])
			
			
			
			