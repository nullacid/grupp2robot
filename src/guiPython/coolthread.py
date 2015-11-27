import threading

#This class is used for timing out the bluetooth connection
class coolThread(threading.Thread):
	def __init__(self, threadID, name, function):
		threading.Thread.__init__(self)
		self.name = name
		self.tid = threadID
		self.mainFunction = function
		
		self.returnValue = None
		self.done = False
		
	def run(self):
		self.returnValue = self.mainFunction()
		self.done = True
		