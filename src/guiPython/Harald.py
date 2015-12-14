"""
	Created: November 2015
 *  Author: Victor T and Peter T
 * "Proof by anology is fraud." - Bjarne Stroustrup

 * This file handles all bluetooth communication with the system.
 * Functions are used in CRAY.py

"""

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
		
		#Connection Status is used for connection indicator in the gui
		self.connectionstatus = 0

		#Attempts to connect until connection is established
		while not self.establishDirectConnection():
			sleep(1)
			pass	
			
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

		#Attempts to connect, throws IOError.
		try:
			self.ourSocket.connect((self.targetDevice, self.port))
			print("Connected to firefly module")
			
			#Clear any data from the old bluetooth connection
			while not self.__doSync():
				pass

			return True
		except IOError:
			print("Failed to connect to firefly module, reattempting to connect")
			return False
				
	#Sends one byte of data to the bluetooth module
	def sendData(self, data):	
		if self.targetDevice != None:
			self.ourSocket.send(data)
			
	#Attempts to receive one byte of data from the bluetooth module, and reconnects to the bluetooth module
	#if it receives false from __attempReceive()
	def receiveData(self):
		if self.targetDevice != None:
			data = self.__attemptReceive()
			if not data:
				while not self.establishDirectConnection():
					sleep(1)
				return b'\xff'
			else:
				return data
				
	#Attempts to receive one byte of data, returns false if it times out, otherwise returns the data
	def __attemptReceive(self):
		data = None
		self.ourSocket.settimeout(4.0)
		try:
			data = self.ourSocket.recv(1)
			return data
		except bluetooth.BluetoothError:
			print("timeout yo")
			return False

	#Sends a sync byte to the communication module in the system. If the system still has data to send from
	#an earlier transmission, these will be swallowed by this function.
	def __doSync(self):
		self.sendData(b'\x26')

		self.ourSocket.settimeout(5.0)

		try:
			while self.receiveData() != b'\x26':
				pass
			return True
		except bluetooth.BluetoothError:
			return False

	#Increments connection status indicator
	def inc_status(self):
		if self.connectionstatus < 3:
			self.connectionstatus += 1
		else:
			self.connectionstatus = 0
		
			
			
			
			
