import pygame
from time import *

#Import bluetooth class
from Harald import *

#Import map system class
from MapSystem import *

#from pygame.locals import *
from pygame import *
import sys, os, traceback



#Center the screen
if sys.platform in ["win32","win64"]: os.environ["SDL_VIDEO_CENTERED"]="1"

	
#Create bluetooth object
harald = Harald()
	
#Initialize
pygame.display.init()
pygame.font.init()

"""#FULLSCREEN MODE
screenWidth = 1600
screenHeight = 900
squareWidth = screenHeight/15
squareHeight = screenHeight/15

screen_size = [screenWidth,screenHeight]

surface = pygame.display.set_mode(screen_size, FULLSCREEN)
offset = -150"""

#SMALLSCREEN MODE
screenWidth = 615
screenHeight = 405
squareWidth = screenHeight/15
squareHeight = screenHeight/15

screen_size = [screenWidth, screenHeight]

surface = pygame.display.set_mode(screen_size)
offset = 0


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


	
def paintData(mapSystem):
	font = pygame.font.Font(None, int((2*squareHeight)/3))
	for i in range(0,17):
		textString = mapSystem.indexDict[i] + ": "
		text = font.render(textString, 0, H4xx0R)
		surface.blit(text, ((6*screenWidth)/9 + offset, i * squareHeight + 10))
		
def paintMap(mapSystem):
	for i in range(0,15):
		for j in range(0,15):
			paintSquare(mapSystem.arrayMap[i][j], i, j)
	pygame.display.flip()
			
def paintSquare(tileType, xCoord, yCoord):
	colour = BLACK
	if tileType == "UNEXPLORED":
		colour = WHITE
	elif tileType == "OPEN":
		colour = BLUE
	elif tileType == "WALL":
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

def handle_BACKSPACE():
	harald.sendData(b'\x09')
	harald.receiveData(1)
	
def handle_SPACE():
	harald.sendData(b'\x09')
	harald.receiveData()
	
	
#Functions for getting data from the system (called autonomously)
#They get data from the system and formats it into data that makes sense for humans.

#Så här kan man göra om en byte till int :)
#print(int(b'\x54'[0]))

def getLidar():
	harald.sendData(b'\x88')
	dataArray =[]
	dataArray.append(harald.receiveData())
	dataArray.append(harald.receiveData())
	#return dataArray
	
def getIRRF():
	harald.sendData(b'\x49')
	data = harald.receiveData()
	return data

def getIRRB():
	harald.sendData(b'\x4A')
	data = harald.receiveData()

def getIRLF():
	harald.sendData(b'\x4B')
	data = harald.receiveData()

def getIRLB():
	harald.sendData(b'\x4C')
	data = harald.receiveData()

def getGyro():
	harald.sendData(b'\x8D')
	dataArray = []
	dataArray.append(harald.receiveData())
	dataArray.append(harald.receiveData())

def getLidarToken():
	harald.sendData(b'\x4F')
	data = harald.receiveData()

def getParallelRight():
	harald.sendData(b'\x50')
	data = harald.receiveData()

def getParallelLeft():
	harald.sendData(b'\x51')
	data = harald.receiveData()
	
def getGyroToken():
	harald.sendData(b'\x52')
	data = harald.receiveData()

def getIRRFtoken():
	harald.sendData(b'\x53')
	data = harald.receiveData()

def getIRRBtoken():
	harald.sendData(b'\x54')
	data = harald.receiveData()

def getIRLFtoken():
	harald.sendData(b'\x55')
	data = harald.receiveData()

def getIRLBtoken():
	harald.sendData(b'\x56')
	data = harald.receiveData()

def getSteering():
	harald.sendData(b'\x59')
	data = harald.receiveData()

def getMap():
	global mapSystem
	harald.sendData(b'\x98')
	dataArray = []
	dataArray.append(harald.receiveData())
	dataArray.append(harald.receiveData())

def getPosition():
	harald.sendData(b'\x9A')
	dataArray = []
	dataArray.append(harald.receiveData())
	dataArray.append(harald.receiveData())

def getDecision():
	harald.sendData(b'\x5B')
	data = harald.receiveData()
	
#dictionary of key bindings for keydown
handle_dictionary_down = {
	K_ESCAPE: handle_quit,
	K_a: left_down,
	K_s: back_down,
	K_w: forward_down,
	K_d: right_down,
	K_BACKSPACE: handle_BACKSPACE,
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
	"IRrightFront (token)" : getIRRFtoken,
	"IRrightBack (token)" : getIRRBtoken,
	"IRleftFront (token)" : getIRLFtoken,
	"IRleftBack (token)" : getIRLBtoken,
	"Steering data" : getSteering,
	"Update Map" : getMap,
	"System Position" : getPosition,
	"Steering Decision" : getDecision		
}

mapSystem = MapSystem();

def getData():
	currentDataSlot = mapSystem.indexDict[mapSystem.dataIndex]
	mapSystem.dataDict[currentDataSlot] = handle_dictionary_data[currentDataSlot]()
	
	mapSystem.incIndex()	#This is essently dataIndex++ but it loops it at 17

	

	
while(crayRunning):
	paintMap(mapSystem)
	paintData(mapSystem)
		
	#getData()
	
	for event in pygame.event.get():
		if event.type == pygame.KEYDOWN and event.key in handle_dictionary_down:
			handle_dictionary_down[event.key]()
		if event.type == pygame.KEYUP and event.key in handle_dictionary_up:
			handle_dictionary_up[event.key]()
	