
class MapSystem():
	def __init__(self):
		self.arrayMap = [["UNEXPLORED" for x in range(33)] for x in range(33)]
		self.startPosition = (16,16)
		self.dataIndex = 0
		"""self.arrayMap[16][16] = "OPEN"
		self.arrayMap[17][17] = "OPEN"
		self.arrayMap[16][17] = "OPEN"
		self.arrayMap[17][16] = "OPEN"
		self.arrayMap[15][16] = "OPEN"
		self.arrayMap[16][15] = "OPEN"
		self.arrayMap[15][15] = "OPEN"
		self.arrayMap[15][17] = "OPEN"
		self.arrayMap[17][15] = "OPEN"
		
		self.arrayMap[15][14] = "WALL"
		self.arrayMap[16][14] = "WALL"
		self.arrayMap[17][14] = "WALL"
		self.arrayMap[18][15] = "WALL"
		self.arrayMap[18][16] = "WALL"
		self.arrayMap[18][17] = "WALL"
		self.arrayMap[15][18] = "WALL"
		self.arrayMap[16][18] = "WALL"
		self.arrayMap[17][18] = "WALL"
		self.arrayMap[14][15] = "WALL"
		self.arrayMap[14][16] = "WALL"
		self.arrayMap[14][17] = "WALL"""

		
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
						17 : "Steering Decision",
						18 : "Debug"
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
						"Steering Decision" : 1,
						"Debug" : 1
		}
	#Increments index and loops it at 17
	def incIndex(self):
		self.dataIndex += 1
		if self.dataIndex > 18:
			self.dataIndex = 0
		