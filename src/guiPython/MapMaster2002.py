

class MapMaster2002():
	def __init__(self):
		self.arrayMap = [["UNEXPLORED" for x in range(32)] for x in range(32)]
		self.startPosition = (15,15)
		self.sysPosX = 15
		self.sysPosY = 15
		self.dataIndex = 0

		self.lastX = 15
		self.lastY = 15

		self.coordinateOffsetX = - 1
		self.coordinateOffsetY = - 1

		self.tileImg = None
		
		#self.gyroFile = open("logs/gyro.swag", 'w', 1)
		self.IRFrontFile = open("logs/ir_front.swag", 'w', 1)
		self.IRtokenFile = open("logs/IRtoken.swag", 'w', 1)
		self.steeringDecisionFile = open("logs/steering_decision.swag", 'w', 1)
		self.steeringDataFile = open("logs/steering_data.swag", 'w', 1)
		self.mapUpdateFile = open("logs/map_updates.swag", 'w', 1)
		self.parallelRightFile = open("logs/parallel_right.swag", 'w', 1)
		self.parallelLeftFile = open("logs/parallel_left.swag", 'w', 1)
		self.debugFile = open("logs/debug.swag", 'w', 1)
		self.irRightFile = open("logs/irRight.swag", 'w', 1)
		self.segmentsFile = open("logs/segments.swag", 'w', 1)
		

		#Dictionary used for looping over data types
		self.indexDict = {
						0 : "IR Front",
						1 : "IRrightFront",
						2 : "IRrightBack",
						3 : "IRleftFront",
						4 : "IRleftBack",
						5 : "Segments turned",
						6 : "IR Front (token)",
						7 : "Parallel Right",
						8 : "Parallel Left",
						#9 : "Gyro (token)",
						9 : "IRright (token)",
						10 : "IRleft (token)",
						11 : "Steering data",
						12 : "Update Map",
						13 : "System Position",
						14 : "Steering Decision",
						15 : "Debug"
		}
		#Store values for each data type
		self.dataDict = {
						"IR Front" : 1,
						"IRrightFront" : 1,
						"IRrightBack" : 1,
						"IRleftFront" : 1,
						"IRleftBack" : 1,
						"Segments turned" : 1,
						"IR Front (token)" : 1,
						"Parallel Right" : 1,
						"Parallel Left" : 1,
						"IRright (token)" : 1,
						"IRleft (token)" : 1,
						"Steering data" : 1,
						"Update Map" : 1,
						"System Position" : 1,
						"Steering Decision" : 1,
						"Debug" : 1
		}
		#Associates a data type name with a file
		self.fileDict = {
						"IR Front" : self.IRFrontFile,
						"Steering Decision" : self.steeringDecisionFile,
						"Steering data" : self.steeringDataFile,
						"Parallel Right" : self.parallelRightFile,
						"Parallel Left" : self.parallelLeftFile,
						"Segments turned" : self.segmentsFile,
						"Debug" : self.debugFile,
						"IR Front (token)" : self.IRtokenFile,
						"Update Map" : self.mapUpdateFile,
						"IRrightFront" : self.irRightFile
		}


	#Increments index and loops it at 17
	def incIndex(self):
		self.dataIndex += 1
		if self.dataIndex > 15:
			self.dataIndex = 0
	
	#Writes the data currently mapped to the input data type to the data type's log file
	def updateLog(self, dataType):
		if dataType in self.fileDict:
			self.fileDict[dataType].write(str(round(time.clock(), 1)) + "s " + str(self.dataDict[dataType]) + "\n")

	#Resizes the map to only include the found area
	def resize(self):
		leftmostWall = 32
		rightmostWall = 0

		upmostWall = 32
		downmostWall = 0

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

		diff = 0
		if downmostWall - upmostWall >= rightmostWall - leftmostWall:
			diff = downmostWall - upmostWall
		else:
			diff = rightmostWall - leftmostWall

		if diff > 5:
			#Create new arrayMap
			newArray = [["UNEXPLORED" for x in range(diff)] for x in range(diff)]
			for row in range(0, diff):
				for col in range(0, diff):
					newArray[col][row] = self.arrayMap[col + leftmostWall][row + upmostWall]


			self.arrayMap = newArray
			self.coordinateOffsetX = self.coordinateOffsetX - leftmostWall
			self.coordinateOffsetY = self.coordinateOffsetY - upmostWall

			self.sysPosX = self.sysPosX - leftmostWall
			self.sysPosY = self.sysPosY - upmostWall
			self.lastX = self.lastX - leftmostWall
			self.lastY = self.lastY - upmostWall

			self.startPosition = (15 - leftmostWall, 15 - upmostWall)
			#Resizingg succesful
			return True
		else:
			#Resizing denied
			return False