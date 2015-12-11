
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


#Debugmode
debug = False

"""#FULLSCREEN MODE
screenWidth = 1536
screenHeight = 864
squareWidth = screenHeight/32
squareHeight = screenHeight/32

screen_size = [screenWidth,screenHeight]

surface = pygame.display.set_mode(screen_size, FULLSCREEN)
offset = -15
#dataOffset = 300

"""#SMALLSCREEN MODE
screenWidth = 960
screenHeight = 640
squareWidth = screenHeight/32
squareHeight = screenHeight/32






screen_size = [screenWidth, screenHeight]

surface = pygame.display.set_mode(screen_size)
offset = -40
#dataOffset = 220"""

tileUNEXPLORED = pygame.transform.scale(pygame.image.load("white_tile.jpg"),(int(squareWidth),int(squareHeight)))
tileOPEN = pygame.transform.scale(pygame.image.load("tile_open.jpg"),(int(squareWidth),int(squareHeight)))
tileWALL = pygame.transform.scale(pygame.image.load("wall_tile.jpg"),(int(squareWidth),int(squareHeight)))
tileLEFTWALL = pygame.transform.scale(pygame.image.load("wall_tile.jpg"), (int(squareWidth), int(squareHeight)))
tileOUTSIDE = pygame.transform.scale(pygame.image.load("white_tile.jpg"),(int(squareWidth),int(squareHeight)))


tile_ship_down = pygame.transform.scale(pygame.image.load("tile_ship_down.png"),(int(squareWidth),int(squareHeight)))
tile_ship_up = pygame.transform.scale(pygame.image.load("tile_ship_up.png"),(int(squareWidth),int(squareHeight)))
tile_ship_left = pygame.transform.scale(pygame.image.load("tile_ship_left.png"),(int(squareWidth),int(squareHeight)))
tile_ship_right = pygame.transform.scale(pygame.image.load("tile_ship_right.png"),(int(squareWidth),int(squareHeight)))
tile_start = pygame.transform.scale(pygame.image.load("tile_start.png"),(int(squareWidth),int(squareHeight)))
#A blank icon
icon = pygame.Surface((1,1)); icon.set_alpha(0); pygame.display.set_icon(icon)
#This caption
pygame.display.set_caption("M/S SEA++ TEST PROGRAM")


#Global variable for when the game is running
crayRunning = True


lastX = 15
lastY = 15
lastTile = tile_ship_up

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
	for i in range(0,16):
		textString = mapSystem.indexDict[i] + ":  " + str(mapSystem.dataDict[mapSystem.indexDict[i]])
		text = font.render(textString, 0, H4xx0R)
		surface.blit(text, (35* squareWidth + offset, i * (5 * squareHeight)/3 + 10))
		
	#Start Position Circle
	
	#surface.blit(tile_start,(15*squareWidth, 15*squareHeight))
	
	pygame.draw.circle(surface, RED, [int(35 * squareWidth), int(17 * (5 * squareHeight) / 3 + 10)], int(squareWidth / 2))
	
	text = "Start Position"
	text = font.render(text, 0 , WHITE)
	surface.blit(text, (36 * squareWidth, 17 * (5 * squareHeight) / 3))
	
	#Current Position Circle
	pygame.draw.circle(surface, MAGENTA, [int(35 * squareWidth), int(18 * (5 * squareHeight) / 3 + 10)], int(squareWidth / 2))
	text = "Current Position"
	text = font.render(text, 0 , WHITE)
	surface.blit(text, (36 * squareWidth, 18 * (5 * squareHeight) / 3))
	
	#Conneciton status
	pygame.draw.rect(surface, GREEN, [screenWidth - 50 + harald.connectionstatus*10, screenHeight - 20, 10, 10])
	
def paintMap(mapSystem):
	for i in range(0,32):
		for j in range(0,32):
			paintSquare(mapSystem.arrayMap[i][j], i, j)

	#Draw startPosition
	#pygame.draw.circle(surface, RED, [int(mapSystem.startPosition[0] * squareWidth + squareWidth / 2), int(mapSystem.startPosition[1] * squareWidth + squareHeight/2)], int(squareWidth / 2))

	surface.blit(tile_start,(int(mapSystem.startPosition[0])*squareWidth, int(mapSystem.startPosition[1])*squareHeight))
	

	#Draw currentPosition
	
	if (lastX == int(mapSystem.sysPosX) and (lastY == int(mapSystem.sysPosY))): #same
		tileImg = lastTile
	elif (lastX == int(mapSystem.sysPosX) and (lastY == (int(mapSystem.sysPosY) +1))): #up
		tileImg = tile_ship_up
		lastTile = tileImg
	elif (lastX == int(mapSystem.sysPosX) and lastY == (int(mapSystem.sysPosY) -1)): #down
		tileImg = tile_ship_down
		lastTile = tileImg
	elif (lastX == (int(mapSystem.sysPosX)+ 1) and lastY == int(mapSystem.sysPosY)): #right
		tileImg = tile_ship_right
		lastTile = tileImg
	elif (lastX == (int(mapSystem.sysPosX) - 1) and lastY == int(mapSystem.sysPosY)): #left
		tileImg = tile_ship_left
		lastTile = tileImg
		
	surface.blit(tile_start,(int(mapSystem.sysPosX)*squareWidth, int(mapSystem.sysPosY*squareHeight)))

	#pygame.draw.circle(surface, MAGENTA, [int(mapSystem.sysPosX * squareWidth + squareWidth / 2), int(mapSystem.sysPosY * squareWidth + squareHeight/2)], int(squareWidth / 2))
	
	lastX = mapSystem.sysPosX
	lastY = mapSystem.sysPosY
	
	
	pygame.display.flip()

	
def paintSquare(tileType, xCoord, yCoord):
	tileImg = tileUNEXPLORED
	if tileType == "UNEXPLORED":
		tileImg = tileUNEXPLORED
	elif tileType == "OPEN":
		tileImg = tileOPEN
	elif tileType == "WALL":
		tileImg = tileWALL
	elif tileType == "OUTSIDE":
		tileImg = tileOUTSIDE
	elif tileType == "LEFT WALL":
		tileImg = tileLEFTWALL

		
	surface.blit(tileImg,(xCoord*squareWidth, yCoord*squareHeight))
	
	#pygame.draw.rect(surface, colour, [xCoord*squareWidth, yCoord*squareHeight, squareWidth, squareHeight])
	
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

def spinL_down():
	harald.sendData(b'\x22')

def spinL_up():
	harald.sendData(b'\x23')

def spinR_down():
	harald.sendData(b'\x24')

def spinR_up():
	harald.sendData(b'\x25')
	
def handle_SPACE():
	pass
	
	
#Functions for getting data from the system (called autonomously)
#They get data from the system and formats it into data that makes sense for humans.

#Så här kan man göra om en byte till int :)
#print(int(b'\x54'[0]))
def getIRF():
	if debug:
		print ("IRF")
	
	harald.sendData(b'\x48')
	data = harald.receiveData()
	return int(data[0])
	
def getIRRF():
	if debug:
		print("IRRF")
	harald.sendData(b'\x49')
	data = harald.receiveData()
	return int(data[0])

def getIRRB():
	if debug:
		print("IRRB")
	harald.sendData(b'\x4A')
	data = harald.receiveData()
	return int(data[0])

def getIRLF():
	if debug:
		print("IRLF")
	harald.sendData(b'\x4B')
	data = harald.receiveData()
	return int(data[0])

def getIRLB():
	if debug:
		print("IRLB")
	harald.sendData(b'\x4C')
	data = harald.receiveData()
	return int(data[0])

#NOT USED ATM
def getGyro():
	if debug:
		print("gyro")
	harald.sendData(b'\x8D')
	msByte = harald.receiveData()
	lsByte = harald.receiveData()
	data = int(msByte[0])*256 + int(lsByte[0])
	return data

def getReflexToken():
	if debug:
		print("reflex token")
	harald.sendData(b'\x57')
	data = int(harald.receiveData()[0])
	if data > 40:
		print("ANOMALY DETECTED")
	return data

	
def getIRFToken():
	if debug:
		print("IRF token")
	harald.sendData(b'\x4F')
	data = harald.receiveData()
	return int(data[0])

def getParallelRight():
	if debug:
		print("parallelRight")
	harald.sendData(b'\x50')
	data = harald.receiveData()
	out = twos_comp(int(hex(data[0]),16), 8)
	return out

def getParallelLeft():
	if debug:
		print("parallelleft")
	harald.sendData(b'\x51')
	data = harald.receiveData()
	out = twos_comp(int(hex(data[0]),16), 8)
	return out
	
def getGyroToken():
	if debug:
		print("gyrotoken")
	harald.sendData(b'\x52')
	data = harald.receiveData()
	return int(data[0])

def getIRRFtoken():
	if debug:
		print("IRRFtoken")
	harald.sendData(b'\x53')
	data = harald.receiveData()
	return int(data[0])

def getIRRBtoken():
	if debug:
		print("IRRBtoken")
	harald.sendData(b'\x54')
	data = harald.receiveData()
	return int(data[0])

def getIRLFtoken():
	if debug:
		print("IRLFtoken")
	harald.sendData(b'\x55')
	data = harald.receiveData()
	return int(data[0])

def getIRLBtoken():
	if debug:
		print("IRLBtoken")
	harald.sendData(b'\x56')
	data = harald.receiveData()
	return int(data[0])

#First byte is left engine, second byte is right engine
def getSteering():
	if debug:
		print("steering")
	harald.sendData(b'\x99')
	leftEngine = int(harald.receiveData()[0])
	rightEngine = int(harald.receiveData()[0])
	return str(leftEngine) + "  " + str(rightEngine)

#Gets data from the map update stack in bjarne and updates the map graphically
def getMap():
	if debug:
		print("map")
	harald.sendData(b'\x98')
	msByte = harald.receiveData()
	lsByte = harald.receiveData()
	xCoord = int(msByte[0]) - 1
	yCoord = int(lsByte[0] & b'\x3F'[0]) - 1
	tileType = int(lsByte[0] >> 6)

	if tileType != 0:
		if tileType == 1:
			tileType = "LEFT WALL"
		elif tileType == 2:
			tileType = "OPEN"
		elif tileType == 3:
			tileType = "WALL"

		if xCoord < 32 and yCoord < 32:
			mapSystem.arrayMap[xCoord][yCoord] = tileType
			return "x: " + str(xCoord) + "; y: " + str(yCoord) + "; " + str(tileType)

	return mapSystem.dataDict["Update Map"]
	

def getPosition():
	if debug:
		print("position")
	harald.sendData(b'\x9A')
	mapSystem.sysPosX = int(harald.receiveData()[0]) - 1
	mapSystem.sysPosY = int(harald.receiveData()[0]) - 1
	return "x: " + str(mapSystem.sysPosX) + "; y: " + str(mapSystem.sysPosY)

def getDecision():
	if debug:
		print("decision")
	harald.sendData(b'\x5B')
	data = harald.receiveData()
	return int(data[0])

	
def getDebug():
	if debug:
		print("debug")
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
	K_q: spinL_down,
	K_e: spinR_down,
	K_SPACE: handle_SPACE
}


#dictionary for key bindings for keyup
handle_dictionary_up = {
	K_w: forward_up,
	K_a: left_up,
	K_s: back_up,
	K_d: right_up,
	K_q: spinL_up,
	K_e: spinR_up
}

#dictionary for binding functions to names of sensor data we want to get, see MapSystem
handle_dictionary_data = {
	"IR Front" : getIRF,
	"IRrightFront" : getIRRF,
	"IRrightBack" : getIRRB,
	"IRleftFront" : getIRLF,
	"IRleftBack" : getIRLB,
	"Segments turned" : getReflexToken,
	"IR Front (token)" : getIRFToken,
	"Parallel Right" : getParallelRight,
	"Parallel Left" : getParallelLeft,
	#"Gyro (token)" : getGyroToken,
	"IRright (token)" : getIRRFtoken,
	"IRleft (token)" : getIRLFtoken,
	"Steering data" : getSteering,
	"Update Map" : getMap,
	"System Position" : getPosition,
	"Steering Decision" : getDecision,
	"Debug" : getDebug
}

#Gets all data from one command and then updates the screen.
#ORDER DATA HAS TO BE TRANSMITTED IN
#NOT CURRENTLY USED
#-------
#IRRF
#IRRB
#IRLF
#IRLB
#IRF
#PR
#PL
#Gt
#IRRt
#IRLt
#IRFt
#Steering
#Map
#SysPos
#Decision
#Debug
#-------
def getAllData():
	#Send command
	harald.sendData(b'\x1D')

	#Get IR sensor data
	mapSystem.dataDict["IRrightFront"] = int(harald.receiveData()[0])
	mapSystem.dataDict["IRrightBack"] = int(harald.receiveData()[0])
	mapSystem.dataDict["IRleftFront"] = int(harald.receiveData()[0])
	mapSystem.dataDict["IRleftBack"] = int(harald.receiveData()[0])
	mapSystem.dataDict["IR Front"] = int(harald.receiveData()[0])

	#Get Parallel tokens
	mapSystem.dataDict["Parallel Right"] = int(harald.receiveData()[0])
	mapSystem.dataDict["Parallel Left"] = int(harald.receiveData()[0])

	#Get gyro token
	mapSystem.dataDict["Gyro (token)"] = int(harald.receiveData()[0])

	#Get IR token data
	mapSystem.dataDict["IRright (token)"] = int(harald.receiveData()[0])
	mapSystem.dataDict["IRleft (token)"] = int(harald.receiveData()[0])
	mapSystem.dataDict["IR Front (token)"] = int(harald.receiveData()[0])

	#Get Steering data. (Output to engine)
	leftSteering = int(harald.receiveData()[0])
	rightSteering = int(harald.receiveData()[0])
	mapSystem.dataDict["Steering data"] = str(leftEngine) + "  " + str(rightEngine)

	#Get Map data from change stack
	msByteMap = harald.receiveData()
	lsByteMap = harald.receiveData()
	xCoordMap = int(msByteMap[0]) - 1
	yCoordMap = int(lsByteMap[0] & b'\x3F'[0]) - 1
	tileType = int(lsByteMap[0] >> 6)
	if tileType != 0:
		if tileType == 1:
			tileType = "UNEXPLORED"
		elif tileType == 2:
			tileType = "OPEN"
		elif tileType == 3:
			tileType = "WALL"
		if xCoord < 32 and yCoord < 32:
			mapSystem.arrayMap[xCoord][yCoord] = tileType
			mapSystem.dataDict["Update Map"] = "x: " + str(xCoord) + "; y: " + str(yCoord) + "; " + str(tileType)

	#Get System Position
	mapSystem.sysPosX = int(harald.receiveData()[0]) - 1
	mapSystem.sysPosY = int(harald.receiveData()[0]) - 1
	mapSystem.dataDict["System Position"] = "x: " + str(mapSystem.sysPosX) + "; y: " + str(mapSystem.sysPosY)

	#Get Steering Decision
	mapSystem.dataDict["Steering Decision"] = int(harald.receiveData()[0])
	#Update the log for steering decision, great for debugging
	mapSystem.updateLog("Steering Decision")

	#Get Debug
	mapSystem.dataDict["Debug"] = int(harald.receiveData()[0])

	#Update Screen
	paintMap(mapSystem)
	paintData(mapSystem)

#Gets one data value from the system (decided by dataIndex in mapSystem)
#Increments dataIndex so that the next data value will be gathered the next time this function is called.
def getData():
	currentDataSlot = mapSystem.indexDict[mapSystem.dataIndex]
	mapSystem.dataDict[currentDataSlot] = handle_dictionary_data[currentDataSlot]()
	mapSystem.updateLog(currentDataSlot)

	#if mapSystem.dataIndex % 2 == 0:
	#	getMap()
	#	getPosition()

	getMap()
	
	if mapSystem.dataIndex == 0:
		harald.inc_status()
		paintMap(mapSystem)
		paintData(mapSystem)

	
	mapSystem.incIndex()	#This is essentially dataIndex++ but it loops it at 17
	

#mainloop
while(crayRunning):
	getData()

	for event in pygame.event.get():
		if event.type == pygame.KEYDOWN and event.key in handle_dictionary_down:
			handle_dictionary_down[event.key]()
		if event.type == pygame.KEYUP and event.key in handle_dictionary_up:
			handle_dictionary_up[event.key]()
