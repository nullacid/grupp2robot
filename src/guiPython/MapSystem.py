import time
class MapSystem():
	def __init__(self):
		self.arrayMap = [["UNEXPLORED" for x in range(33)] for x in range(33)]
		self.startPosition = (16,16)
		self.dataIndex = 0
		
		self.gyroFile = open("logs/gyro.swag", 'w', 1)
		self.lidarFile = open("logs/lidar.swag", 'w', 1)
		self.steeringDecisionFile = open("logs/steering_decision.swag", 'w', 1)
		self.ParallelRightFile = open("logs/parallel_right.swag", 'w', 1)
		self.debugFile = open("logs/debug.swag", 'w', 1)
		self.lidartokenfile = open("logs/lidartoken.swag", 'w', 1)

		#Dictionary used for looping over data types
		self.indexDict = {
						0 : "Lidar",
						1 : "IRrightFront",
						2 : "IRrightBack",
						3 : "IRleftFront",
						4 : "IRleftBack",
						5 : "Gyro",
						6 : "Lidar (token)",
						7 : "Parallel Right",
						8 : "Parallel Left",
						9 : "Gyro (token)",
						10 : "IRright (token)",
						11 : "IRleft (token)",
						12 : "Steering data",
						13 : "Update Map",
						14 : "System Position",
						15 : "Steering Decision",
						16 : "Debug"
		}
		#Store values for each data type
		self.dataDict = {
						"Lidar" : 1,
						"IRrightFront" : 1,
						"IRrightBack" : 1,
						"IRleftFront" : 1,
						"IRleftBack" : 1,
						"Gyro" : 1,
						"Lidar (token)" : 1,
						"Parallel Right" : 1,
						"Parallel Left" : 1,
						"Gyro (token)" : 1,
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
						"Lidar" : self.lidarFile,
						"Gyro" : self.gyroFile,
						"Steering Decision" : self.steeringDecisionFile,
						"Parallel Right" : self.ParallelRightFile,
						"Debug" : self.debugFile,
						"Lidar (token)" : self.lidartokenfile
		}
	#Increments index and loops it at 17
	def incIndex(self):
		self.dataIndex += 1
		if self.dataIndex > 16:
			self.dataIndex = 0
	
	#Writes the data currently mapped to the input data type to the data type's log file
	def updateLog(self, dataType):
		if dataType in self.fileDict:
			self.fileDict[dataType].write(str(round(time.clock(), 1))+ " " + str(self.dataDict[dataType]) + "\n")
		