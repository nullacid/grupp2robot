import pygame
import bluetooth

#from pygame.locals import *
from pygame import *
import sys, os, traceback

#Mac Address of our firefly module
fireflyMacAddr = '00:06:66:03:A6:96'

#Center the screen
if sys.platform in ["win32","win64"]: os.environ["SDL_VIDEO_CENTERED"]="1"

print("performing inquiry...")
nearby_devices = []
try:
	nearby_devices = bluetooth.discover_devices(
		duration=8, lookup_names=True, flush_cache=True, lookup_class=False)
except IOError:
	print("Can't find any bluetooth devices")

print("found %d devices" % len(nearby_devices))
for addr, name in nearby_devices:
	try:
		print("  %s - %s" % (addr, name))
	except UnicodeEncodeError:
		print("  %s - %s" % (addr, name.encode('utf-8', 'replace')))
	
	

#Find the desired firefly module in the list of nearby_devices	
desiredDevice = None		
for i in range(0, len(nearby_devices)):
	if nearby_devices[i][0] == fireflyMacAddr:
		desiredDevice = nearby_devices[i][0]
	
#Setup connection with firefly module
if desiredDevice != None:
	ourSock = bluetooth.BluetoothSocket(bluetooth.RFCOMM)
	port = 1
	ourSock.connect((desiredDevice, port))
	#data = input("please input data: ")
	#ourSock.send(data)
	#print("data sent")
	print("Connected")
	while True:
		data = ourSock.recv(1)
		print(data)
	
	
	
	ourSock.close()
	
	
	


#Initialize
#pygame.display.init()
#pygame.font.init()

#Set up a window
screenWidth = 1600
screenHeight = 900
squareWidth = screenHeight/15
squareHeight = screenHeight/15

screen_size = [screenWidth,screenHeight]
#A blank icon
#icon = pygame.Surface((1,1)); icon.set_alpha(0); pygame.display.set_icon(icon)
#This caption
#pygame.display.set_caption("M/S SEA++ TEST PROGRAM")
#Make the windowing surface!
#surface = pygame.display.set_mode(screen_size, FULLSCREEN)

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
	
#paints a rectangle on the board
def handle_A():
	global robotMap
	robotMap.arrayMap[1][5] = "OPEN"
	
def handle_W():
	global robotMap
	robotMap.arrayMap[9][4] = "WALL"
	
def handle_S():
	global robotMap
	robotMap.arrayMap[1][5] = "UNEXPLORED"
	
def handle_D():
	global robotMap
	robotMap.arrayMap[9][4] = "UNEXPLORED"

#clears the board
def handle_BACKSPACE():
	global robotMap
	robotMap.clearMap()
	
def handle_SPACE():
	global robotMap
	robotMap.drawMap()

	
#dictionary of key bindings
handle_dictionary = {
	K_ESCAPE: handle_quit,
	K_a: handle_A,
	K_s: handle_S,
	K_w: handle_W,
	K_d: handle_D,
	K_BACKSPACE: handle_BACKSPACE,
	K_SPACE: handle_SPACE
}



robotMap = RobotMap();

#while(crayRunning):
	#robotMap.paintMap()
	#robotMap.paintText()
	#for event in pygame.event.get():
	#	if event.type == pygame.KEYDOWN and event.key in handle_dictionary:
	#		handle_dictionary[event.key]()