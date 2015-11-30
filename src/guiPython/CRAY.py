
import pygame
#Import bluetooth class
from Harald import *

#Import map system class
from MapSystem import *

#from pygame.locals import *
from pygame import *


import sys, os, traceback

from time import *


#Center the screen
if sys.platform in ["win32","win64"]: os.environ["SDL_VIDEO_CENTERED"]="1"

	

#Create bluetooth object
harald = Harald()

mapSystem = MapSystem()
	
#Initialize
pygame.display.init()
pygame.font.init()


"""#FULLSCREEN MODE
screenWidth = 1584
screenHeight = 891
squareWidth = screenHeight/33
squareHeight = screenHeight/33

screen_size = [screenWidth,screenHeight]

surface = pygame.display.set_mode(screen_size, FULLSCREEN)
offset = -15
#dataOffset = 300

"""#SMALLSCREEN MODE
screenWidth = 990
screenHeight = 660
squareWidth = screenHeight/33
squareHeight = screenHeight/33

screen_size = [screenWidth, screenHeight]

surface = pygame.display.set_mode(screen_size)
offset = -10
#dataOffset = 220"""



#A blank icon
icon = pygame.Surface((1,1)); icon.set_alpha(0); pygame.display.set_icon(icon)
#This caption
pygame.display.set_caption("M/S SEA++ TEST PROGRAM")


#Global variable for when the game is running
crayRunning = True

#Define RGB colours
BLACK = (0,0,0)
WHITE = (255,255,255)
BLUE = (0,0,255)
GREEN = (0,255,0)
H4xx0R = (173,255,47)
RED = (255,0,0)
CYAN = (0,255,255)
MAGENTA = (255,0,255)
YELLOW = (255,255,0)
GREY = (139, 137, 137)


	
def paintData(mapSystem):
	pygame.draw.rect(surface, BLACK, [15*squareWidth, 0, screenWidth - (15*squareWidth), screenHeight])
	font = pygame.font.Font(None, int((3*squareHeight)/2))
	for i in range(0,17):
		textString = mapSystem.indexDict[i] + ":  " + str(mapSystem.dataDict[mapSystem.indexDict[i]])
		text = font.render(textString, 0, H4xx0R)
		surface.blit(text, (35* squareWidth + offset, i * (5 * squareHeight)/3 + 10))
		
	#Start Position Circle
	pygame.draw.circle(surface, RED, [int(35 * squareWidth), int(18 * (5 * squareHeight) / 3 + 10)], int(squareWidth / 2))
	text = "Start Position"
	text = font.render(text, 0 , WHITE)
	surface.blit(text, (36 * squareWidth, 18 * (5 * squareHeight) / 3))
	
	#Current Position Circle
	pygame.draw.circle(surface, MAGENTA, [int(35 * squareWidth), int(19 * (5 * squareHeight) / 3 + 10)], int(squareWidth / 2))
	text = "Current Position"
	text = font.render(text, 0 , WHITE)
	surface.blit(text, (36 * squareWidth, 19 * (5 * squareHeight) / 3))
	
	#Conneciton status
	pygame.draw.rect(surface, GREEN, [screenWidth - 50 + harald.connectionstatus*10, screenHeight - 20, 10, 10])
	
def paintMap(mapSystem):
	for i in range(0,33):
		for j in range(0,33):
			paintSquare(mapSystem.arrayMap[i][j], i, j)

	#Draw startPosition
	pygame.draw.circle(surface, RED, [int(mapSystem.startPosition[0] * squareWidth + squareWidth / 2), int(mapSystem.startPosition[1] * squareWidth + squareHeight/2)], int(squareWidth / 2))
	
	#Draw currentPosition
	pygame.draw.circle(surface, MAGENTA, [int(mapSystem.sysPosX * squareWidth + squareWidth / 2), int(mapSystem.sysPosY * squareWidth + squareHeight/2)], int(squareWidth / 2))
	
	pygame.display.flip()
			
def paintSquare(tileType, xCoord, yCoord):
	colour = BLACK
	if tileType == "UNEXPLORED":
		colour = GREY
	elif tileType == "OPEN":
		colour = WHITE
	elif tileType == "WALL":
		colour = GREEN
	elif tileType == "OUTSIDE":
		colour = MAGENTA
	pygame.draw.rect(surface, colour, [xCoord*squareWidth, yCoord*squareHeight, squareWidth, squareHeight])
	
	
#key binding handles
def handle_quit():
	global crayRunning
	harald.ourSocket.close()
	crayRunning = False
	pygame.quit()
	
#Functions for steering the system (called with keybindings)
def left_down():
	harald.sendData(b'\x02')

def back_down():
	harald.sendData(b'\x04')

def forward_down():
	harald.sendData(b'\x00')

def right_down():
	harald.sendData(b'\x06')

def left_up():
	harald.sendData(b'\x03')

def back_up():
	harald.sendData(b'\x05')

def forward_up():
	harald.sendData(b'\x01')

def right_up():
	harald.sendData(b'\x07')
	
def handle_SPACE():
	pass
	
	
#Functions for getting data from the system (called autonomously)
#They get data from the system and formats it into data that makes sense for humans.

#Så här kan man göra om en byte till int :)
#print(int(b'\x54'[0]))

def getLidar():
	harald.sendData(b'\x88')
	msByte = harald.receiveData()
	lsByte = harald.receiveData()
	data = int(msByte[0])*256 + int(lsByte[0])
	return data
	
def getIRRF():
	harald.sendData(b'\x49')
	data = harald.receiveData()
	return int(data[0])

def getIRRB():
	harald.sendData(b'\x4A')
	data = harald.receiveData()
	return int(data[0])

def getIRLF():
	harald.sendData(b'\x4B')
	data = harald.receiveData()
	return int(data[0])

def getIRLB():
	harald.sendData(b'\x4C')
	data = harald.receiveData()
	return int(data[0])

def getGyro():
	harald.sendData(b'\x8D')
	msByte = harald.receiveData()
	lsByte = harald.receiveData()
	data = int(msByte[0])*256 + int(lsByte[0])
	return data

def getLidarToken():
	harald.sendData(b'\x4F')
	data = harald.receiveData()
	return int(data[0])

def getParallelRight():
	harald.sendData(b'\x50')
	data = harald.receiveData()
	out = twos_comp(int(hex(data[0]),16), 8)
	return out

def getParallelLeft():
	harald.sendData(b'\x51')
	data = harald.receiveData()
	out = twos_comp(int(hex(data[0]),16), 8)
	return out
	
def getGyroToken():
	harald.sendData(b'\x52')
	data = harald.receiveData()
	return int(data[0])

def getIRRFtoken():
	harald.sendData(b'\x53')
	data = harald.receiveData()
	return int(data[0])

def getIRRBtoken():
	harald.sendData(b'\x54')
	data = harald.receiveData()
	return int(data[0])

def getIRLFtoken():
	harald.sendData(b'\x55')
	data = harald.receiveData()
	return int(data[0])

def getIRLBtoken():
	harald.sendData(b'\x56')
	data = harald.receiveData()
	return int(data[0])

#First byte is left engine, second byte is right engine
def getSteering():
	harald.sendData(b'\x99')
	leftEngine = int(harald.receiveData()[0])
	rightEngine = int(harald.receiveData()[0])
	return str(leftEngine) + "  " + str(rightEngine)

#Gets data from the map update stack in bjarne and updates the map graphically
def getMap():
	harald.sendData(b'\x98')
	msByte = harald.receiveData()
	lsByte = harald.receiveData()
	xCoord = int(msByte[0])
	yCoord = int(lsByte[0] & b'\x3F'[0])
	tileType = int(lsByte[0] >> 6)
	if tileType != 0:
		if tileType == 1:
			tileType = "UNEXPLORED"
		elif tileType == 2:
			tileType = "OPEN"
		elif tileType == 3:
			tileType = "WALL"
	
		mapSystem.arrayMap[xCoord][yCoord] = tileType
		return "x: " + str(xCoord) + "; y: " + str(yCoord) + "; " + str(tileType)
	return mapSystem.dataDict["Update Map"]
	

def getPosition():
	harald.sendData(b'\x9A')
	mapSystem.sysPosX = int(harald.receiveData()[0])
	mapSystem.sysPosY = int(harald.receiveData()[0])
	return "x: " + str(mapSystem.sysPosX) + "; y: " + str(mapSystem.sysPosY)

def getDecision():
	""""
	harald.sendData(b'\x5B')
	data = harald.receiveData()
	return int(data[0])
	"""
	pass
	
def getDebug():
	harald.sendData(b'\x4E')
	data = harald.receiveData()
	out = twos_comp(int(hex(data[0]),16), 8)
	return out
	
def twos_comp(val, bits):
    if (val & (1 << (bits - 1))) != 0: # if sign bit is set e.g., 8bit: 128-255
        val = val - (1 << bits)        # compute negative value
    return val                         # return positive value as is

#dictionary of key bindings for keydown
handle_dictionary_down = {
	K_ESCAPE: handle_quit,
	K_a: left_down,
	K_s: back_down,
	K_w: forward_down,
	K_d: right_down,
	K_SPACE: handle_SPACE
}


#dictionary for key bindings for keyup
handle_dictionary_up = {
	K_w: forward_up,
	K_a: left_up,
	K_s: back_up,
	K_d: right_up
}

#dictionary for binding functions to names of sensor data we want to get, see MapSystem
handle_dictionary_data = {
	"Lidar" : getLidar,
	"IRrightFront" : getIRRF,
	"IRrightBack" : getIRRB,
	"IRleftFront" : getIRLF,
	"IRleftBack" : getIRLB,
	"Gyro" : getGyro,
	"Lidar (token)" : getLidarToken,
	"Parallel Right" : getParallelRight,
	"Parallel Left" : getParallelLeft,
	"Gyro (token)" : getGyroToken,
	"IRright (token)" : getIRRFtoken,
	"IRleft (token)" : getIRLFtoken,
	"Steering data" : getSteering,
	"Update Map" : getMap,
	"System Position" : getPosition,
	"Steering Decision" : getDecision,
	"Debug" : getDebug
}
timestamp = clock()

#Gets one data value from the system (decided by dataIndex in mapSystem)
#Increments dataIndex so that the next data value will be gathered the next time this function is called.
def getData():
	global timestamp
	currentDataSlot = mapSystem.indexDict[mapSystem.dataIndex]
	mapSystem.dataDict[currentDataSlot] = handle_dictionary_data[currentDataSlot]()
	mapSystem.updateLog(currentDataSlot)
	
	if mapSystem.dataIndex == 0:
		paintMap(mapSystem)
		paintData(mapSystem)
		print(str(round(clock() - timestamp, 1)))
		timestamp = clock()
	
	mapSystem.incIndex()	#This is essentially dataIndex++ but it loops it at 17
	


while(crayRunning):
	getData()
	
	for event in pygame.event.get():
		if event.type == pygame.KEYDOWN and event.key in handle_dictionary_down:
			handle_dictionary_down[event.key]()
		if event.type == pygame.KEYUP and event.key in handle_dictionary_up:
			handle_dictionary_up[event.key]()
