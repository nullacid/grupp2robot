

class MapSystem():
	def __init__(self):
		self.arrayMap = [["UNEXPLORED" for x in range(15)] for x in range(15)]
		self.dataIndex = 0
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
						10 : "IRrightFront (token)",
						11 : "IRrightBack (token)",
						12 : "IRleftFront (token)",
						13 : "IRleftBack (token)",
						14 : "Steering data",
						15 : "Update Map",
						16 : "System Position",
						17 : "Steering Decision"		
		}
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
						"IRrightFront (token)" : 1,
						"IRrightBack (token)" : 1,
						"IRleftFront (token)" : 1,
						"IRleftBack (token)" : 1,
						"Steering data" : 1,
						"Update Map" : 1,
						"System Position" : 1,
						"Steering Decision" : 1			
		}
	#Increments index and loops it at 17
	def incIndex(self):
		self.dataIndex += 1
		if self.dataIndex > 17:
			self.dataIndex = 0
		