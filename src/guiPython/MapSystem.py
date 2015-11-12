

class MapSystem(self):
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
						9 : "IRrightFront (token)",
						10 : "IRrightBack (token)",
						11 : "IRleftFront (token)",
						12 : "IRleftBack (token)",
						13 : "Steering data",
						14 : "Update Map",
						15 : "System Position",
						16 : "Steering Decision"		
		}
		self.dataDict = {
						"Lidar" : 0,
						"IRrightFront" : 0,
						"IRrightBack" : 0,
						"IRleftFront" : 0,
						"IRleftBack" : 0,
						"Gyro" : 0,
						"Lidar (token)" : 0,
						"Parallel Right" : 0,
						"Parallel Left" : 0,
						"IRrightFront (token)" : 0,
						"IRrightBack (token)" : 0,
						"IRleftFront (token)" : 0,
						"IRleftBack (token)" : 0,
						"Steering data" : 0,
						"Update Map" : 0,
						"System Position" : 0,
						"Steering Decision" : 0			
		}
		