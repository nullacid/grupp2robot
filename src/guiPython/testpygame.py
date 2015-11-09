import pygame
from time import *

#Import bluetooth class
from Harald import *

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



class RobotMap():
	def __init__(self):
		self.arrayMap = [["UNEXPLORED" for x in range(15)] for x in range(15)]	
		
	def paintMap(self):
		for i in range(0,15):
			for j in range(0,15):
				self.__paintSquare(robotMap.arrayMap[i][j], i, j)
		pygame.display.flip()
		
	def paintText(self):
		font = pygame.font.Font(None, 36)
		text = font.render("tjabba", 0, WHITE)
		surface.blit(text, ((7*screenWidth)/9, screenHeight/2))
		
		
	def __paintSquare(self, squareType, i , j):
		colour = BLACK
		if squareType == "UNEXPLORED":
			colour = WHITE
		elif squareType == "OPEN":
			colour = BLUE
		elif squareType == "WALL":
			colour = MAGENTA
		pygame.draw.rect(surface, colour, [i*squareWidth, j*squareHeight, squareWidth, squareHeight])
		
	def clearMap(self):
		self.arrayMap = [["UNEXPLORED" for x in range(15)] for x in range(15)]	
					
#key binding handles
def handle_quit():
	global crayRunning
	crayRunning = False
	pygame.quit()
	
def left_down():
	global harald
	harald.sendData(b'\x02')

def back_down():
	global harald
	harald.sendData(b'\x04')

def forward_down():
	global harald
	harald.sendData(b'\x80')

def right_down():
	global harald
	harald.sendData(b'\x86')

def left_up():
	global harald
	harald.sendData(b'\x83')

def back_up():
	global harald
	harald.sendData(b'\x85')

def forward_up():
	global harald
	harald.sendData(b'\x01')

def right_up():
	global harald
	harald.sendData(b'\x07')

def handle_BACKSPACE():
	global harald
	harald.sendData(b'\x89')
	print("Received data: " + harald.waitToReceive(1))
	
def handle_SPACE():
	global harald
	harald.sendData(b'\x01')
	data = harald.receiveData(1)
	print("Data received: " + hex(data[0]));
	
	
#dictionary of key bindings
handle_dictionary_down = {
	K_ESCAPE: handle_quit,
	K_a: left_down,
	K_s: back_down,
	K_w: forward_down,
	K_d: right_down,
	K_BACKSPACE: handle_BACKSPACE,
	K_SPACE: handle_SPACE
}

handle_dictionary_up = {
	K_w: forward_up,
	K_a: left_up,
	K_s: back_up,
	K_d: right_up
}

robotMap = RobotMap();

while(crayRunning):
	robotMap.paintMap()
	robotMap.paintText()
	for event in pygame.event.get():
		if event.type == pygame.KEYDOWN and event.key in handle_dictionary_down:
			handle_dictionary_down[event.key]()
		if event.type == pygame.KEYUP and event.key in handle_dictionary_up:
			handle_dictionary_up[event.key]()