"""
	Created: November 2015
 *  Author: Peter T and Victor T
 * "I have not told half of what I saw." - Marco Polo 

 * This class handles CRAY's representation of the map and the system's position.
 * It also stores all data that the system has to offer.

"""

import time

class MapMaster2002():
	def __init__(self):

		#2d-array that the map is stored in. The different tiles are
		#"UNEXPLORED"
		#"OPEN"
		#"WALL"
		#"LEFT WALL" 
		self.arrayMap = [["UNEXPLORED" for x in range(32)] for x in range(32)]


		self.startPosition = (15,15)

		#System's current position
		self.sysPosX = 15
		self.sysPosY = 15
		
		#System's position from the last time it was gathered from the system.
		#Used for choosing the direction of the sprite in the GUI.
		self.lastX = 15
		self.lastY = 15

		#Offset in coordinates from the values sent by the system.
		#Initiated as -1 because the system has a larger array for its map (for pathfinding).
		self.coordinateOffsetX = - 1
		self.coordinateOffsetY = - 1

		#The sprite used for painting the system. Set in CRAY.py
		self.tileImg = None

		#Index used for iterating through the different sensor values.
		self.dataIndex = 0
		
		#Data Files used for logging data
		#Data will only be logged if self.logData is set to True
		self.IRFrontFile = open("logs/ir_front.swag", 'w', 1)
		self.IRRFfile = open("logs/irrf.swag", 'w', 1)
		self.IRRBfile = open("logs/irrb.swag", 'w', 1)
		self.IRLFfile = open("logs/irlf.swag", 'w', 1)
		self.IRLBfile = open("logs/irlb.swag", 'w', 1)
		self.segmentsFile = open("logs/segments.swag", 'w', 1)

		self.systemPosFile = open("logs/sysPos.swag", 'w', 1)

		self.steeringDecisionFile = open("logs/steering_decision.swag", 'w', 1)
		self.steeringDataFile = open("logs/steering_data.swag", 'w', 1)
		self.debugFile = open("logs/debug.swag", 'w', 1)

		self.logData = False
		

		#Dictionary used for looping over data types
		self.indexDict = {
						0 : "IR Front",
						1 : "IRrightFront",
						2 : "IRrightBack",
						3 : "IRleftFront",
						4 : "IRleftBack",
						5 : "Distance Covered",
						6 : "Steering data",
						#7 : "Steering Decision",
						7 : "System Position",
						8 : "Update Map"#,

						#10 : "IR Front (token)",
						#11 : "Parallel Right",
						#12 : "Parallel Left",
						#13 : "IRright (token)",
						#14 : "IRleft (token)",

						#8 : "Debug"
		}
		#Store values for each data type, the keys correspond to the values in indexDict
		#Some of these values are not used anymore.
		self.dataDict = {
						"IR Front" : 0,
						"IRrightFront" : 0,
						"IRrightBack" : 0,
						"IRleftFront" : 0,
						"IRleftBack" : 0,
						"Distance Covered" : 0,
						"IR Front (token)" : 0,
						"Parallel Right" : 0,
						"Parallel Left" : 0,
						"IRright (token)" : 0,
						"IRleft (token)" : 0,
						"Steering data" : 0,
						"Update Map" : 0,
						"System Position" : 0,
						"Steering Decision" : 0,
						"Debug" : 0
		}
		#Associates a data type name with a file
		self.fileDict = {
						"IR Front" : self.IRFrontFile,
						"IRrightFront" : self.IRRFfile,
						"IRrightBack" : self.IRRBfile,
						"IRleftFront" : self.IRLFfile,
						"IRleftBack" : self.IRLBfile,
						"Steering Decision" : self.steeringDecisionFile,
						"Steering data" : self.steeringDataFile,
						"Distance Covered" : self.segmentsFile,
						"Debug" : self.debugFile,
						"System Position" : self.systemPosFile
		}


	#Increments index and loops it at the end of indexDict.
	def incIndex(self):
		self.dataIndex += 1
		if self.dataIndex >= len(self.indexDict):
			self.dataIndex = 0
	
	#Writes the data currently mapped to the input data type to the data type's log file
	#Won't do anything if self.logData is set to False
	def updateLog(self, dataType):
		if dataType in self.fileDict and self.logData:
			self.fileDict[dataType].write(str(round(time.clock(), 1)) + "s " + str(self.dataDict[dataType]) + "\n")

	#Resizes the map to only include the found area
	def resize(self):
		#Values used to compare which wall is the furthest in a specific direction.
		#Initially set to the opposite direction's max value i.e. the worst value it can have.
		leftmostWall = 32
		rightmostWall = 0
		upmostWall = 32
		downmostWall = 0

		#Finds the leftmost and rightmost wall
		for row in range(0,len(self.arrayMap)):
			firstFound = False
			lastFound = False
			for col in range(0,len(self.arrayMap)):
				if not firstFound:
					if self.arrayMap[col][row] != "UNEXPLORED":
						firstFound = True
						if col < leftmostWall:
							leftmostWall = col
				elif not lastFound:
					if self.arrayMap[col][row] == "UNEXPLORED":
						lastFound = True
						if col > rightmostWall:
							rightmostWall = col

		#Finds the upmost and downmost wall
		for col in range(0,len(self.arrayMap)):
			firstFound = False
			lastFound = False
			for row in range(0,len(self.arrayMap)):
				if not firstFound:
					if self.arrayMap[col][row] != "UNEXPLORED":
						firstFound = True
						if row < upmostWall:
							upmostWall = row
				elif not lastFound:
					if self.arrayMap[col][row] == "UNEXPLORED":
						lastFound = True
						if row > downmostWall:
							downmostWall = row

		#Finds the largest difference between x and y coordinate to determine the size of the resized map
		diff = 0
		if downmostWall - upmostWall >= rightmostWall - leftmostWall:
			diff = downmostWall - upmostWall
		else:
			diff = rightmostWall - leftmostWall

		#Create new arrayMap with the new size and creates a deep copy of the old array.
		newArray = [["UNEXPLORED" for x in range(diff)] for x in range(diff)]
		for row in range(0, diff):
			for col in range(0, diff):
				newArray[col][row] = self.arrayMap[col + leftmostWall][row + upmostWall]

		self.arrayMap = newArray

		#Change the offset values so that new map (and position) data will be placed correctly
		self.coordinateOffsetX = self.coordinateOffsetX - leftmostWall
		self.coordinateOffsetY = self.coordinateOffsetY - upmostWall

		#Updates system position to the new map.
		self.sysPosX = self.sysPosX - leftmostWall
		self.sysPosY = self.sysPosY - upmostWall
		self.lastX = self.lastX - leftmostWall
		self.lastY = self.lastY - upmostWall
		self.startPosition = (15 - leftmostWall, 15 - upmostWall)
