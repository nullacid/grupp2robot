"""
	Created: November 2015
 *  Author: Peter T and Victor T
 * "It is not fair to ask of others what you are unwilling to do yourself." - Victor 

 * Main file for the CRAY super computer.

"""

import pygame
from pygame import *

from math import *

#Import bluetooth class
from Harald import *

#Import map system class
from MapMaster2002 import *


import sys, os, traceback

#Center the screen
if sys.platform in ["win32","win64"]: os.environ["SDL_VIDEO_CENTERED"]="1"

fullscreenMode = False



#Create bluetooth object
harald = Harald()

#Create map object
mapSystem = MapMaster2002()
	
#Initialize
pygame.display.init()
pygame.font.init()

#Declare screen related values
screenWidth = None
screenHeight = None
squareWidth = None
squareHeight = None

screen_size = None

surface = None
offset = None

#Initialize screen values to fullscreen mode
if fullscreenMode:
	screenWidth = 1536
	screenHeight = 864
	squareWidth = screenHeight/32
	squareHeight = screenHeight/32

	screen_size = [screenWidth,screenHeight]

	surface = pygame.display.set_mode(screen_size, FULLSCREEN)
	offset = -15

#Initialize screen values to smaller screen mode
else:
	screenWidth = 960
	screenHeight = 640
	squareWidth = screenHeight/32
	squareHeight = screenHeight/32

	screen_size = [screenWidth, screenHeight]

	surface = pygame.display.set_mode(screen_size)
	offset = -40

#These values are changed when map size is changed, some parts of the GUI use these values
#Others used squareWidth/Height
mapSquareWidth = squareWidth
mapSquareHeight = squareHeight

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

#Makes white pixels transparent
surface.set_colorkey(WHITE)

#Values used for storing images to be painted on the GUI. set in rescale()
tileUNEXPLORED = None
tileOPEN = None
tileWALL = None
tileLEFTWALL = None
tileOUTSIDE = None

tile_ship_down = None
tile_ship_up = None
tile_ship_left = None
tile_ship_right = None
tile_start = None



#Rescales all images to the current squareSize
def rescale():
	global tileUNEXPLORED
	global tileOPEN
	global tileWALL
	global tileLEFTWALL
	global tile_ship_right
	global tile_ship_up
	global tile_ship_left
	global tile_ship_down
	global tile_start

	tileUNEXPLORED = pygame.transform.smoothscale(pygame.image.load("images/white_tile_test.jpg"),(int(mapSquareWidth),int(mapSquareHeight)))
	tileOPEN = pygame.transform.smoothscale(pygame.image.load("images/tile_open.jpg"),(int(mapSquareWidth),int(mapSquareHeight)))
	tileWALL = pygame.transform.smoothscale(pygame.image.load("images/wall_tile.jpg"),(int(mapSquareWidth),int(mapSquareHeight)))
	tileLEFTWALL = pygame.transform.smoothscale(pygame.image.load("images/tile_wall_left.jpg"), (int(mapSquareWidth), int(mapSquareHeight)))

	tile_ship_down = pygame.transform.scale(pygame.image.load("images/tile_ship_down.png"),(int(mapSquareWidth),int(mapSquareHeight)))
	tile_ship_up = pygame.transform.scale(pygame.image.load("images/tile_ship_up.png"),(int(mapSquareWidth),int(mapSquareHeight)))
	tile_ship_left = pygame.transform.scale(pygame.image.load("images/tile_ship_left.png"),(int(mapSquareWidth),int(mapSquareHeight)))
	tile_ship_right = pygame.transform.scale(pygame.image.load("images/tile_ship_right.png"),(int(mapSquareWidth),int(mapSquareHeight)))
	tile_start = pygame.transform.scale(pygame.image.load("images/tile_start.png"),(int(mapSquareWidth),int(mapSquareHeight)))

#Sets special sizes for the markers used in the data sections (these won't change when map changes size)
tile_ship_right_data = pygame.transform.scale(pygame.image.load("images/tile_ship_right.png"),(int(squareWidth),int(squareHeight)))
tile_start_data = pygame.transform.scale(pygame.image.load("images/tile_start.png"),(int(squareWidth),int(squareHeight)))

#Sets the tiles.
rescale()

#Sets the initial tile to be painted for system position
mapSystem.tileImg = tile_ship_up



#Removes the silly pygame icon from the pygame window
icon = pygame.Surface((1,1)); icon.set_alpha(0); pygame.display.set_icon(icon)
#Sets window name
pygame.display.set_caption("CRAY™ Super Computer: Connected to M/S Sea++")


#Global variable for when the game is running
crayRunning = True


#Paints the data field with all sensor data
def paintData(mapSystem):
	pygame.draw.rect(surface, BLACK, [20*squareWidth, 0, screenWidth - (15*squareWidth), screenHeight])

	font = pygame.font.Font(None, int((3*squareHeight)/2))

	#Loops through all data values (disregarding some values that are presented other ways)
	for i in range(0, len(mapSystem.indexDict) - 2):
		textString = mapSystem.indexDict[i] + ":  " + str(mapSystem.dataDict[mapSystem.indexDict[i]])
		text = font.render(textString, 0, H4xx0R)
		surface.blit(text, (35* squareWidth + offset, i * (5 * squareHeight)/3 + 10))
		
	#Start position symbol for legend
	surface.blit(tile_start_data, (34*squareWidth, int(17 * (5 * squareHeight) / 3)))

	text = "Start Position"
	text = font.render(text, 0 , WHITE)
	surface.blit(text, (36 * squareWidth, 17 * (5 * squareHeight) / 3))
	
	#Current position symbol for legend
	surface.blit(tile_ship_right_data, (34*squareWidth, int(18 * (5 * squareHeight) / 3)))
	text = "Current Position"
	text = font.render(text, 0 , WHITE)
	surface.blit(text, (36 * squareWidth, 18 * (5 * squareHeight) / 3))
	
	#Connection status indicator
	pygame.draw.rect(surface, GREEN, [screenWidth - 50 + harald.connectionstatus*10, screenHeight - 20, 10, 10])
	
#Paints the entire map
def paintMap(mapSystem):
	for i in range(0,len(mapSystem.arrayMap)):
		for j in range(0,len(mapSystem.arrayMap)):
			paintSquare(mapSystem.arrayMap[i][j], i, j)

	#Draw startPosition
	surface.blit(tile_start,(int(mapSystem.startPosition[0])*mapSquareWidth, int(mapSystem.startPosition[1])*mapSquareHeight))

	#Draw currentPosition, compares last coordinates with current coordinates to determine direction
	if mapSystem.lastX == mapSystem.sysPosX + 1 and mapSystem.lastY == mapSystem.sysPosY:
		mapSystem.tileImg = tile_ship_left
	elif mapSystem.lastX == mapSystem.sysPosX - 1 and mapSystem.lastY == mapSystem.sysPosY:
		mapSystem.tileImg = tile_ship_right
	elif mapSystem.lastX == mapSystem.sysPosX and mapSystem.lastY == mapSystem.sysPosY + 1:
		mapSystem.tileImg = tile_ship_up
	elif mapSystem.lastX == mapSystem.sysPosX and mapSystem.lastY == mapSystem.sysPosY - 1:
		mapSystem.tileImg = tile_ship_down
 
		
	surface.blit(mapSystem.tileImg,(int(mapSystem.sysPosX)*mapSquareWidth, int(mapSystem.sysPosY*mapSquareHeight)))

	lastX = mapSystem.sysPosX
	lastY = mapSystem.sysPosY
	
	#Update Screen
	pygame.display.flip()

#Paints a square give tile type and coordinates.
def paintSquare(tileType, xCoord, yCoord):
	tileImg = tileUNEXPLORED
	if tileType == "UNEXPLORED":
		tileImg = tileUNEXPLORED
	elif tileType == "OPEN":
		tileImg = tileOPEN
	elif tileType == "WALL":
		tileImg = tileWALL
	elif tileType == "LEFT WALL":
		tileImg = tileLEFTWALL

	surface.blit(tileImg,(xCoord*mapSquareWidth, yCoord*mapSquareHeight))
	
	
#Key binding handle for quitting the program (ESCAPE)
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
	
#dictionary of key bindings for keydown
handle_dictionary_down = {
	K_ESCAPE: handle_quit,
	K_a: left_down,
	K_s: back_down,
	K_w: forward_down,
	K_d: right_down,
	K_q: spinL_down,
	K_e: spinR_down,
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
	
	
#Functions for getting data from the system (called autonomously)
#They get data from the system and formats it into data that makes sense for humans.
#Functions marked as (not gathered anymore) were primarily used for debugging and are not used anymore.

#IR Front Sensor
def getIRF():	
	harald.sendData(b'\x48')
	return int(harald.receiveData()[0])
	
#IR Right Front Sensor
def getIRRF():
	harald.sendData(b'\x49')
	return int(harald.receiveData()[0])

#IR Right Back Sensor
def getIRRB():
	harald.sendData(b'\x4A')
	return int(harald.receiveData()[0])

#IR Left Front Sensor
def getIRLF():
	harald.sendData(b'\x4B')
	return int(harald.receiveData()[0])

#IR Left Back Sensor
def getIRLB():
	harald.sendData(b'\x4C')
	return int(harald.receiveData()[0])

#Covered Distance Token
def getCoveredDistance():
	harald.sendData(b'\x8D')
	return int(harald.receiveData()[0])*256 + int(harald.receiveData()[0])

#IR Front Token (not gathered anymore)
def getIRFToken():
	harald.sendData(b'\x4F')
	return int(harald.receiveData()[0])

#Parallel Right Token (not gathered anymore)
def getParallelRight():
	harald.sendData(b'\x50')
	data = harald.receiveData()
	return twos_comp(int(hex(data[0]),16), 8)

#Parallel Left Token (not gathered anymore)
def getParallelLeft():
	harald.sendData(b'\x51')
	data = harald.receiveData()
	return twos_comp(int(hex(data[0]),16), 8)
	
#IR Right Front Token (not gathered anymore)
def getIRRFtoken():
	harald.sendData(b'\x53')
	return int(harald.receiveData()[0])

#IR Right Back Token (not gathered anymore)
def getIRRBtoken():
	harald.sendData(b'\x54')
	return int(harald.receiveData()[0])

#IR Left Front Token (not gathered anymore)
def getIRLFtoken():
	harald.sendData(b'\x55')
	return int(harald.receiveData()[0])

#IR Left Back Token (not gathered anymore)
def getIRLBtoken():
	harald.sendData(b'\x56')
	return int(harald.receiveData()[0])

#Steering Data
#First byte is left engine, second byte is right engine
def getSteering():
	harald.sendData(b'\x99')
	leftEngine = int(harald.receiveData()[0])
	rightEngine = int(harald.receiveData()[0])
	return str(leftEngine) + "  " + str(rightEngine)

#Gets data from the map update stack in bjarne and updates the map graphically.
#Resizes the map when system has finished mapping the outer wall
def getMap():
	harald.sendData(b'\x98')
	msByte = harald.receiveData()
	lsByte = harald.receiveData()
	#Special value sent when system has decided that outer wall is done
	if msByte == b'\xFF' and lsByte == b'\xFF':
			global mapSquareWidth
			global mapSquareHeight
			#resizes map
			mapSystem.resize()
			mapSquareWidth = screenHeight/len(mapSystem.arrayMap)
			mapSquareHeight = screenHeight/len(mapSystem.arrayMap)
			#rescales images
			rescale()

	#Special value sent when system has decided that it is finished
	if msByte == b'\xEE' and lsByte == b'\xEE':
		mapSystem.confirmMap()
		pygame.mixer.init()
		pygame.mixer.music.load("sounds/SEGER.wav")
		pygame.mixer.music.play(loops = 0, start = 0.0)

	#Regular map data. Sent in format
	# --xx xxxx - ttyy yyyy (- nothing, x xcoordinate, y ycoordinate, t tiletype)
	else:
		xCoord = int(msByte[0]) + mapSystem.coordinateOffsetX
		yCoord = int(lsByte[0] & b'\x3F'[0]) + mapSystem.coordinateOffsetY
		tileType = int(lsByte[0] >> 6)

		if tileType != 0:
			if tileType == 1:
				tileType = "LEFT WALL"
			elif tileType == 2:
				tileType = "OPEN"
			elif tileType == 3:
				tileType = "WALL"
			#Adds the tile to the mapsystem
			if xCoord < len(mapSystem.arrayMap) and yCoord < len(mapSystem.arrayMap):
				if mapSystem.arrayMap[xCoord][yCoord] != "WALL":
					mapSystem.arrayMap[xCoord][yCoord] = tileType
					return "x: " + str(xCoord) + "; y: " + str(yCoord) + "; " + str(tileType)

		return mapSystem.dataDict["Update Map"]
	
#System Position
def getPosition():
	harald.sendData(b'\x9A')

	mapSystem.lastX = mapSystem.sysPosX
	mapSystem.lastY = mapSystem.sysPosY

	mapSystem.sysPosX = int(harald.receiveData()[0]) + mapSystem.coordinateOffsetX
	mapSystem.sysPosY = int(harald.receiveData()[0]) + mapSystem.coordinateOffsetY
	return "x: " + str(mapSystem.sysPosX) + "; y: " + str(mapSystem.sysPosY)

#Get Steering Decision (not gathered anymore)
def getDecision():
	harald.sendData(b'\x5B')
	data = int(harald.receiveData()[0])
	if data == 0:
		data = "EMPTY"
	elif data == 1:
		data = "FORWARD"
	elif data == 2:
		data = "SPIN R"
	elif data == 3:
		data = "SPIN L"
	elif data == 4:
		data = "SPIN 180"
	elif data == 5:
		data = "PARAL"
	elif data == 6:
		data = "BACKWARD"
	elif data == 7:
		data = "NUDGE F"
	elif data == 8:
		data = "P WEAK"
	elif data == 9:
		data = "P WEAK L"
	elif data == 10:
		data = "NUDGE T W"
	return data
	
#Get Debug value (can be set to whatever in bjarne) (not gathered anymore)
def getDebug():
	harald.sendData(b'\x4E')
	return int(harald.receiveData()[0])

#Gets the twos complement value of a value
def twos_comp(val, bits):
    if (val & (1 << (bits - 1))) != 0: # if sign bit is set e.g., 8bit: 128-255
        val = val - (1 << bits)        # compute negative value
    return val                         # return positive value as is



#dictionary for binding functions to names of sensor data we want to get, see MapSystem
handle_dictionary_data = {
	"IR Front" : getIRF,
	"IRrightFront" : getIRRF,
	"IRrightBack" : getIRRB,
	"IRleftFront" : getIRLF,
	"IRleftBack" : getIRLB,
	"Distance Covered" : getCoveredDistance,
	"IR Front (token)" : getIRFToken,
	"Parallel Right" : getParallelRight,
	"Parallel Left" : getParallelLeft,
	"IRright (token)" : getIRRFtoken,
	"IRleft (token)" : getIRLFtoken,
	"Steering data" : getSteering,
	"Update Map" : getMap,
	"System Position" : getPosition,
	"Steering Decision" : getDecision,
	"Debug" : getDebug
}


#Gets one data value from the system (decided by dataIndex in mapSystem)
#Increments dataIndex so that the next data value will be gathered the next time this function is called.
def getData():
	currentDataSlot = mapSystem.indexDict[mapSystem.dataIndex]
	mapSystem.dataDict[currentDataSlot] = handle_dictionary_data[currentDataSlot]()
	mapSystem.updateLog(currentDataSlot)

	#Update Map more often to keep up with the system. 
	if mapSystem.dataIndex % 2 == 0:
		getMap()

	if mapSystem.dataIndex == len(mapSystem.indexDict) - 1:
		harald.inc_status()		#Update connection status indicator
		paintMap(mapSystem)
		paintData(mapSystem)

	mapSystem.incIndex()
	

#mainloop
while(crayRunning):
	getData()

	for event in pygame.event.get():
		if event.type == pygame.KEYDOWN and event.key in handle_dictionary_down:
			handle_dictionary_down[event.key]()
		if event.type == pygame.KEYUP and event.key in handle_dictionary_up:
			handle_dictionary_up[event.key]()
