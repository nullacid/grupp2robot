

class MapSystem(self):
	def __init__(self):
		self.arrayMap = [["UNEXPLORED" for x in range(15)] for x in range(15)]
		
		self.dataList = []
		self.dataIndex = 0
		self.dataMap = {
						Lidar : 0,
						IRrightFront : 0,
						IRrightBack : 0,
						IRleftFront : 0,
						IRleftBack : 0
						
						}
		