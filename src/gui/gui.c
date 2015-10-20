#include <SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>
#include <time.h>

#define bool int;
#define true 1;
#define false 0;

//Some constants

const int WORLD_WIDTH = 32;
const int WORLD_HEIGHT = 32;
const int INFO_AREA_WIDTH = 300;
const int CAMERA_WIDTH = 640;
const int CAMERA_HEIGHT = 480;
const int TILE_SIZE = 50;
const int SCREEN_WIDTH = (CAMERA_WIDTH+INFO_AREA_WIDTH); //To fit map + border + info area
const int SCREEN_HEIGHT = (CAMERA_HEIGHT); //To fit map + border
const int MAP_AREA_WIDTH = (SCREEN_WIDTH-INFO_AREA_WIDTH);
const int FRAMETIME = 17; //Frametime for 60 fps
const int INFO_PADDING_RIGHT = 40; //Padding between left side of infobox and text
const int INFO_PADDING_VERT = 10;

struct player{

	static const int ROBOT_WIDTH = 30;
	static const int ROBOT_HEIGHT = 30;
	
	int speed;

	int x;
	int y;	
	SDL_Texture* plTex;
	
};

//Some globals
SDL_Window* gWindow = NULL;
SDL_Surface* gWindowSurface = NULL;
SDL_Renderer* gRenderer = NULL;

SDL_Texture* gSampleMap = NULL;
SDL_Texture* gSampleInfo = NULL;
SDL_Texture* tMapBg = NULL;
SDL_Texture* tInfoBg = NULL;
SDL_Texture* tUnknown = NULL;
SDL_Texture* tFloor = NULL;
SDL_Texture* tWall = NULL;


Mix_Music *sMusic = NULL;

int lastTime = 0; //Used for timer
int a = 255;
int aDir = 1;
struct player* pl1 = NULL;
SDL_Surface* gInfoHeader = NULL;


TTF_Font* Sans = NULL;
TTF_Font* mapFont = NULL;
SDL_Color Red = { .r = 255, .g = 0, .b = 0};

int** world;

//Map area viewport
SDL_Rect mapView;


//Info area viewport
SDL_Rect infoView;

//Level camera
SDL_Rect camera;
int camera_x;
int camera_y;


//Prototypes
int init(); //Inits SDL and creates window
void renderMap(); //Renders the map area
void renderInfo(); //Renders the info area
void renderPlayer(); //Renders players
void renderText();
void close(); //Cleans up and exits program
SDL_Texture* loadTex(char* filename, int r, int g, int b); //Loads media from filename
void loadMedia();
void placeText(int x, int y, TTF_Font* font, char* msg, SDL_Color color);
void centerCameraOnPlayer();



int main(int argc, char* args[]){

	if(!init()){
		printf("Failed to init :((((((( \n");
	}
	else{
		printf("init done!\n");
		//Set up
		int running = true;
		SDL_Event e;
		
		mapView.x = 0;
		mapView.y = 0;
		mapView.w = MAP_AREA_WIDTH;
		mapView.h = SCREEN_HEIGHT;

		infoView.x = MAP_AREA_WIDTH;
		infoView.y = 0;
		infoView.w = INFO_AREA_WIDTH;
		infoView.h = SCREEN_HEIGHT;
				
		
		while(running){ //Main loop
			
			
			while(SDL_PollEvent(&e) != 0){ //Handle event queue
				
				switch(e.type){
				
					case SDL_QUIT: //Window X is pressed
						running = false;
						break;
					case SDL_KEYDOWN:
						switch(e.key.keysym.sym){
						
							case SDLK_w:							
								printf("MOVE FORWARD FAGGOT\n");
								pl1->y -= pl1->speed;							
							break;
							case SDLK_a:							
								printf("MOVE FORWARD FAGGOT\n");
								pl1->x -= pl1->speed;							
							break;
							case SDLK_s:							
								printf("MOVE FORWARD FAGGOT\n");
								pl1->y += pl1->speed;							
							break;
							case SDLK_d:							
								printf("MOVE FORWARD FAGGOT\n");
								pl1->x += pl1->speed;							
							break;
						
						}
						break;
				
				}
			
			}			
			int currentTime = SDL_GetTicks();
			if (currentTime > lastTime + FRAMETIME) {
				
				if(a == 0){
					aDir = 1;
				}
				if(a == 255){
					aDir = -1;
				}
				
				a = a + (aDir);
				
				SDL_SetTextureAlphaMod(gSampleMap, a);

			
				SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF );
				SDL_RenderClear( gRenderer );		
				renderMap();
				renderInfo();	
				renderPlayer();
				renderText();
				//Update Screen
				SDL_RenderPresent(gRenderer);
				
				lastTime = currentTime;				
			}
			
		}
		
	}
	
	close();

}

void renderText(){


	SDL_RenderSetViewport(gRenderer, &infoView);	

	placeText(INFO_PADDING_RIGHT, INFO_PADDING_VERT, Sans, "DICKS <33", Red);
	placeText(INFO_PADDING_RIGHT, INFO_PADDING_VERT*3, mapFont, "Yar har!", Red);

}

void placeText(int x, int y, TTF_Font* font, char* msg, SDL_Color color){

	SDL_Surface* messageSurface = TTF_RenderText_Solid(font, msg, color);	
	SDL_Texture* gMessage = SDL_CreateTextureFromSurface(gRenderer, messageSurface);	
	SDL_Rect gRenderRect= {.x = x, .y = y, .w = messageSurface->w, .h = messageSurface->h};
	SDL_RenderCopy(gRenderer, gMessage, NULL, &gRenderRect);

	SDL_FreeSurface(messageSurface);
	SDL_DestroyTexture(gMessage);

	return;
	
}

void renderPlayer(){

	centerCameraOnPlayer();
	SDL_RenderSetViewport(gRenderer, &mapView);
	
	SDL_Rect renderQuad;
	
	
	renderQuad.x = (pl1->x - camera_x);
	renderQuad.y = (pl1->y - camera_y);
	renderQuad.w = TILE_SIZE;
	renderQuad.h = TILE_SIZE;
	
	SDL_RenderCopy(gRenderer, pl1->plTex, NULL, &renderQuad);

}

void renderMap(){

	//Select viewport
	SDL_RenderSetViewport(gRenderer, &mapView);	

	//Render the bg
	
	SDL_Rect gRenderRect = {.x = -camera_x, .y = -camera_y, .w = WORLD_WIDTH*TILE_SIZE, .h = WORLD_HEIGHT*TILE_SIZE};
	SDL_RenderCopy(gRenderer, tMapBg, NULL, &gRenderRect);
	
	
	//Render the world 
	for(int i = 0; i < WORLD_HEIGHT; i++){
		for (int j = 0; j < WORLD_WIDTH; j++){
			
			if((i*TILE_SIZE)){ //check if current tile is inside the camera view;
			
				SDL_Rect gRenderRect = {.x = i*TILE_SIZE - camera_x, .y = j*TILE_SIZE - camera_y, .w = TILE_SIZE, .h = TILE_SIZE};
			
				switch(world[i][j]){
				
					case 0:
					
						SDL_SetTextureAlphaMod( tUnknown, rand()%255);
						SDL_RenderCopy(gRenderer, tUnknown, NULL, &gRenderRect);
						break;
						
					case 1:
					
						SDL_RenderCopy(gRenderer, tFloor, NULL, &gRenderRect);
						break;
						
					case 2:
					
						SDL_RenderCopy(gRenderer, tWall, NULL, &gRenderRect);
						break;			
					
				}
			}
			
		}
	}
	
	
}

void renderInfo(){

	//Select viewport
	SDL_RenderSetViewport(gRenderer, &infoView);
	
	//Get texture width
	int tWidth = 0;
	SDL_QueryTexture(tInfoBg, NULL, NULL, &tWidth, NULL);
		
	SDL_Rect gRenderRect = {.x = 0, .y = 0, .w = tWidth, .h = SCREEN_HEIGHT };

	
	SDL_RenderCopy(gRenderer, tInfoBg, NULL, &gRenderRect);
}

int init(){


	srand(time(NULL));
	int success = true;
	SDL_Init(SDL_INIT_EVERYTHING);
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");		
	gWindow = SDL_CreateWindow( "SpelPunktC", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN );		
	gRenderer = SDL_CreateRenderer( gWindow, -1, SDL_RENDERER_ACCELERATED );
	SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
	TTF_Init();
	Mix_OpenAudio( 44100, MIX_DEFAULT_FORMAT, 2, 2048 );
	
	loadMedia();
	
	pl1 = (struct player*)malloc(sizeof(struct player));
	printf("Player address bithc: %d \n", pl1);
	pl1->x = 100;
	pl1->y = 50;
	pl1->plTex = loadTex("res/hello.bmp", 0, 0, 0xFF);
	pl1->speed = 10;
	printf("pl1 x: %d\n",pl1->x);
	
	Mix_PlayMusic(sMusic, -1);
	
	
	camera = {.x = 0, .y = 0, .w = CAMERA_WIDTH, .h = CAMERA_HEIGHT};
	centerCameraOnPlayer();
	
	world = (int**)malloc(WORLD_HEIGHT * sizeof(int*));
	for(int i = 0; i < WORLD_HEIGHT; i++){	
		world[i] = (int*)malloc(WORLD_WIDTH*sizeof(int));	
	}
	
	for (int i = 0; i < WORLD_HEIGHT; i++) {
		for (int j = 0; j < WORLD_WIDTH; j++){
			world[i][j] = rand()%3; 
		}
	}
	
	return success;
	
}

void centerCameraOnPlayer(){

	camera_x = (pl1->x + (pl1->ROBOT_WIDTH/2) - (CAMERA_WIDTH/2));
	camera_y = (pl1->y + (pl1->ROBOT_HEIGHT/2) - (CAMERA_HEIGHT/2));

	 //Keep the camera in bounds
    if( camera_x < 0 ){ 
	
        camera_x = 0;
    }
	
    if( camera_y < 0 ){
	
        camera_y = 0;
    }
	
    if( camera_x > WORLD_WIDTH*TILE_SIZE - CAMERA_WIDTH ){
	
        camera_x = WORLD_WIDTH*TILE_SIZE - CAMERA_WIDTH;
    }
	
    if( camera_y > WORLD_HEIGHT*TILE_SIZE - CAMERA_HEIGHT ){
	
        camera_y = WORLD_HEIGHT*TILE_SIZE - CAMERA_HEIGHT;
    }

}

SDL_Texture* loadFromRenderedText(char* text, SDL_Color textColor){

	SDL_Surface* textSurface = TTF_RenderText_Solid(Sans, text, textColor);
	SDL_Texture* mTexture = SDL_CreateTextureFromSurface(gRenderer, textSurface);
	SDL_FreeSurface(textSurface);
	
	return mTexture;


}

void loadMedia(){

	tInfoBg = loadTex("res/infobg.bmp",0,0,0xFF);
	SDL_SetTextureBlendMode(tInfoBg, SDL_BLENDMODE_BLEND);	

	tMapBg = loadTex("res/mapbg.bmp",0,0,0xFF);
	SDL_SetTextureBlendMode(tMapBg, SDL_BLENDMODE_BLEND);	
	
	tUnknown = loadTex("res/water.bmp",0,0,0xFF);
	SDL_SetTextureBlendMode(tUnknown, SDL_BLENDMODE_BLEND);	
	
	tFloor = loadTex("res/wall.bmp",0,0,0xFF);
	SDL_SetTextureBlendMode(tWall, SDL_BLENDMODE_BLEND);	
	
	tWall = loadTex("res/land.bmp",0,0,0xFF);
	SDL_SetTextureBlendMode(tWall, SDL_BLENDMODE_BLEND);	
	
	
	Sans = TTF_OpenFont("res/Sans.ttf", 24);
	mapFont = TTF_OpenFont("res/Treamd.ttf", 24);	


	sMusic = Mix_LoadMUS("res/ost.mp3");

}


SDL_Texture* loadTex(char* filename, int r, int g, int b){

	SDL_Texture* newTex = NULL;
	SDL_Surface* loadedSurface = SDL_LoadBMP(filename);
	
	if(loadedSurface == NULL){
	
		printf("Loading of %s failed! Error: %s \n", filename, SDL_GetError());
	
	}
	else{
	
		//Set transparent color
		SDL_SetColorKey( loadedSurface, SDL_TRUE, SDL_MapRGB( loadedSurface->format, r, g, b) );

	
		//Create texture from pixels
		newTex = SDL_CreateTextureFromSurface(gRenderer, loadedSurface);
		
		if (newTex == NULL){
		
			printf("Unable to create texture from %s, error: %s \n", filename, SDL_GetError());
		
		}
		
		//Free old loaded surface
		SDL_FreeSurface(loadedSurface);
	
	}
	
	return newTex;
}


void close(){

	//SDL_DestroyTexture( texpointer)
	//texpointer = NULL;
	
	SDL_DestroyRenderer(gRenderer);
	SDL_DestroyWindow(gWindow);
	gRenderer = NULL;
	gWindow = NULL;
	free(pl1);
	
	for(int i = 0; i < WORLD_HEIGHT; i++){	
		free(world[i]);	
	}
	world = NULL;

	
	Mix_FreeMusic(sMusic);
	sMusic = NULL;
	
	
	SDL_Quit();
}
