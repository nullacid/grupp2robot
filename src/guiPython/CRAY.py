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

#Set up a window
screenWidth = 600
screenHeight = 405
squareWidth = screenHeight/15
squareHeight = screenHeight/15

screen_size = [screenWidth,screenHeight]
#A blank icon
icon = pygame.Surface((1,1)); icon.set_alpha(0); pygame.display.set_icon(icon)
#This caption
pygame.display.set_caption("M/S SEA++ TEST PROGRAM")
#Make the windowing surface!
#surface = pygame.display.set_mode(screen_size, FULLSCREEN)
surface = pygame.display.set_mode(screen_size)

#Global variable for when the game is running
crayRunning = True

#Define colours used
BLACK = (0,0,0)
WHITE = (255,255,255)
BLUE = (0,0,255)
GREEN = (0,255,0)
RED = (255,0,0)
CYAN = (0,255,255)
MAGENTA = (255,0,255)
YELLOW = (255,255,0)


def paintText():
	font = pygame.font.Font(None, 36)
	text = font.render("tjabba", 0, WHITE)
	surface.blit(text, ((7*screenWidth)/9, screenHeight/2))
		
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
	harald.receiveData(1)
	
	
#Functions for getting data from the system (called autonomously)
def getLidar():
	harald.sendData(b'\x88')
	
def getIRRF():
	harald.sendData(b'\x49')

def getIRRB():
	harald.sendData(b'\x4A')

def getIRLF():
	harald.sendData(b'\x4B')

def getIRLB():
	harald.sendData(b'\x4C')

def getGyro():
	harald.sendData(b'\x8D')

def getLidarToken():
	harald.sendData(b'\x4F')

def getParallelRight():
	harald.sendData(b'\x50')

def getParallelLeft():
	harald.sendData(b'\x51')
	
def getGyroToken():
	harald.sendData(b'\x52')

def getIRRFtoken():
	harald.sendData(b'\x53')

def getIRRBtoken():
	harald.sendData(b'\x54')

def getIRLFtoken():
	harald.sendData(b'\x55')

def getIRLBtoken():
	harald.sendData(b'\x56')

def getSteering():
	harald.sendData(b'\x59')

def getMap():
	harald.sendData(b'\x98')

def getPosition():
	harald.sendData(b'\x9A')

def getDecision():
	harald.sendData(b'\x5B')
	
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
	
	#This is essently dataIndex++ but it loops it at 17
	mapSystem.incIndex() 

while(crayRunning):
	paintMap(mapSystem)
	paintText()
	
	#getData()
	
	for event in pygame.event.get():
		if event.type == pygame.KEYDOWN and event.key in handle_dictionary_down:
			handle_dictionary_down[event.key]()
		if event.type == pygame.KEYUP and event.key in handle_dictionary_up:
			handle_dictionary_up[event.key]()
	