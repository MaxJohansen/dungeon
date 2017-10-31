#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>

// Local includes
#include "map.h"
#include "tile.h"
#include "tiletypes.h"
#include "msg_module.h"

// Function declarations
void log_SDL_error(FILE *output, char *message);
void handle_keypress(SDL_Event event);
void get_input(SDL_Event event);
SDL_Texture *image_loader(const char *filename, SDL_Renderer* renderer);
void renderTextureatXY(SDL_Texture *texture, SDL_Renderer *renderer, int x, int y, int size);
void draw_map_hud(SDL_Texture **textures, 
    SDL_Renderer *renderer, 
    int herox, int heroy, 
    map_t *newmap, 
    int MAP_WIDTH, int MAP_HEIGHT, 
    int TILE_SIZE, 
    tiletype_t* map_hud_tiles);
void draw_message_hud_border(SDL_Texture **textures, 
    SDL_Renderer *renderer, 
    int startx, int starty, 
    int MESSAGE_WIDTH, int MESSAGE_HEIGHT, 
    int TILE_SIZE);
void draw_messages(msg_module_t * message_module);


// Let the Dungeon begin!
int main(int argc, char** argv){
    int WINDOW_WIDTH;
    int WINDOW_HEIGHT;
    int TILE_SIZE = 16;

    if(argc != 3){
        printf("Usage: ./dungeon <width> <height>\nUsing Width: 1600 Height: 800\n");
        WINDOW_WIDTH = 1600;
        WINDOW_HEIGHT = 800;
    } else {
        // Window size in pixels
        WINDOW_WIDTH = atoi(argv[1]) - (atoi(argv[1]) % TILE_SIZE);
        WINDOW_HEIGHT = atoi(argv[2]) - (atoi(argv[2]) % TILE_SIZE);
        printf("Width: %d\nHeight: %d\n", WINDOW_WIDTH, WINDOW_HEIGHT);
    }

    // Window size in tiles
    int WINDOW_WIDTH_TILES = WINDOW_WIDTH/TILE_SIZE;
    int WINDOW_HEIGHT_TILES = WINDOW_HEIGHT/TILE_SIZE;

    // Map, hud and minimap sizes.
    int MAP_WIDTH = WINDOW_WIDTH_TILES * 0.7;
    if(MAP_WIDTH % 2 == 0)
        MAP_WIDTH++;
    int MAP_HEIGHT = WINDOW_HEIGHT_TILES * 0.8;
    if(MAP_HEIGHT % 2 == 0)
        MAP_HEIGHT++;
    // Hero position
    int herox = MAP_WIDTH/2;
    int heroy = MAP_HEIGHT/2;
    //int HUD_WIDTH = WINDOW_WIDTH_TILES;
    //int HUD_HEIGHT = WINDOW_HEIGHT_TILES - MAP_HEIGHT;
    int MESSAGE_WIDTH = WINDOW_WIDTH_TILES - MAP_WIDTH;
    int MESSAGE_HEIGHT = MAP_HEIGHT;

    // Initialize SDL and sub-libraries
    if(SDL_Init(SDL_INIT_VIDEO) < 0){
        log_SDL_error(stdout, "SDL_Init");
        exit(1);
    }
    if(TTF_Init() < 0){
        printf("TTF_Error: %s\n", TTF_GetError());
        exit(1);
    }
    // load support for the JPG and PNG image formats
    if(IMG_Init(IMG_INIT_JPG|IMG_INIT_PNG) < 0) {
        printf("IMG_Init: Failed to init required jpg and png support!\n");
        printf("IMG_Init: %s\n", IMG_GetError());
        // handle error
    }

    SDL_Window *window = SDL_CreateWindow("Dungeon",
                                          SDL_WINDOWPOS_UNDEFINED,
                                          SDL_WINDOWPOS_UNDEFINED,
                                          WINDOW_WIDTH, WINDOW_HEIGHT,
                                          SDL_WINDOW_OPENGL);
    if(NULL == window){
        log_SDL_error(stdout, "SDL_Window");
        exit(1);
    }
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if(NULL == renderer){
        log_SDL_error(stdout, "SDL_Renderer");
        exit(1);
    }
    // Load some images
    SDL_Texture **textures = malloc(TILENUM * sizeof(SDL_Texture*));
    int i;
    for(i = 0; i < TILENUM; i++){
        textures[i] = image_loader(imagefiles[i], renderer);
    }


    // NEW MAP MODULE!
    map_t* newmap = map_create(50, 50);
    // Make an array to hold the tiletypes that gets printed in the map hud.
    tiletype_t *map_hud_tiles = malloc(MAP_HEIGHT * MAP_WIDTH * sizeof(tiletype_t));
    for(i = 0; i < MAP_HEIGHT * MAP_WIDTH; i++){
        map_hud_tiles[i] = DEFAULT;
    }
    // NEW MESSAGE MODULE
    msg_module_t* message_module = msg_module_create(MAP_WIDTH*TILE_SIZE+TILE_SIZE, TILE_SIZE, MESSAGE_WIDTH-(TILE_SIZE*2), MESSAGE_HEIGHT-(TILE_SIZE*2), 5, renderer);

    // Game loop
    int done = 0;
    SDL_Event event;
    while(!done){
        while(SDL_PollEvent(&event)){
            if((event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_q) || event.type == SDL_QUIT){
                printf("Got QUIT.\n");
                done = 1;
                break;
            }
            if(event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_m){
                printf("MAP_WIDTH: %d\nMAP_HEIGHT: %d\n", MAP_WIDTH, MAP_HEIGHT);
                continue;
            }
            if(event.type == SDL_KEYDOWN){
                switch(event.key.keysym.sym)
                {
                case SDLK_KP_7:
                    heroy -= 1;
                    herox -= 1;
                    break;
                case SDLK_KP_8:
                case SDLK_UP:
                    heroy -= 1;
                    break;
                case SDLK_KP_9:
                    heroy -= 1;
                    herox += 1;
                    break;
                case SDLK_KP_2:
                case SDLK_DOWN:
                    heroy += 1;
                    break;
                case SDLK_KP_1:
                    heroy += 1;
                    herox -= 1;
                    break;
                case SDLK_KP_4:
                case SDLK_LEFT:
                    herox -= 1;
                    break;
                case SDLK_KP_6:
                case SDLK_RIGHT:
                    herox += 1;
                    break;
                case SDLK_KP_3:
                    heroy += 1;
                    herox += 1;
                    break;
                default:
                    continue;
                }
            }
            // get_input(event);
        }
        // Clear
        SDL_RenderClear(renderer);

        // Update current_map_hud

        // Draw map-hud
        draw_map_hud(textures, renderer, herox, heroy, newmap, MAP_WIDTH, MAP_HEIGHT, TILE_SIZE, map_hud_tiles);
        // Draw message-hud-border
        draw_message_hud_border(textures, renderer, MAP_WIDTH, 0, MESSAGE_WIDTH, MESSAGE_HEIGHT, TILE_SIZE);
        // If we have new messages, display them.
        if(msg_module_updated(message_module)){
            draw_messages(message_module);
        }
        // Present
        SDL_RenderPresent(renderer);
    }
    // Cleanup and close
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    TTF_Quit();
    SDL_Quit();
}

// --------------------------------------------------
//                  FUNCTIONS
// --------------------------------------------------

void draw_map_hud(SDL_Texture **textures, SDL_Renderer *renderer, int herox, int heroy, map_t *newmap, int MAP_WIDTH, int MAP_HEIGHT, int TILE_SIZE, tiletype_t* map_hud_tiles){
    int x, y;
    int index = 0;
    // update the tiles in the map hud.
    map_get_hud(herox, heroy, MAP_WIDTH, MAP_HEIGHT, map_hud_tiles, newmap);

    for(y = 0; y < MAP_HEIGHT; y++){
        for(x = 0; x < MAP_WIDTH; x++){
            tiletype_t type = map_hud_tiles[index];
            renderTextureatXY(textures[type], renderer, x, y, TILE_SIZE);
            index++;
        }
    }
}


void draw_message_hud_border(SDL_Texture **textures, SDL_Renderer *renderer, int startx, int starty, int MESSAGE_WIDTH, int MESSAGE_HEIGHT, int TILE_SIZE){
    // Draw corners
    renderTextureatXY(textures[B_TOPLEFT], renderer, startx, starty, TILE_SIZE);
    renderTextureatXY(textures[B_TOPRIGHT], renderer, startx + MESSAGE_WIDTH-1, starty, TILE_SIZE);
    renderTextureatXY(textures[B_BOTTOMLEFT], renderer, startx, starty + MESSAGE_HEIGHT-1, TILE_SIZE);
    renderTextureatXY(textures[B_BOTTOMRIGHT], renderer, startx + MESSAGE_WIDTH-1, starty + MESSAGE_HEIGHT-1, TILE_SIZE);
    int x;
    int y;
    // Draw horizontals    
    for(y = starty+1; y < MESSAGE_HEIGHT - 1; y++){
        renderTextureatXY(textures[B_VERTICAL], renderer, startx, y, TILE_SIZE);
        renderTextureatXY(textures[B_VERTICAL], renderer, startx+MESSAGE_WIDTH-1, y, TILE_SIZE);        
    }

    for(x = startx+1; x < startx + MESSAGE_WIDTH - 1; x++){
        renderTextureatXY(textures[B_HORIZONTAL], renderer, x, starty, TILE_SIZE);
        renderTextureatXY(textures[B_HORIZONTAL], renderer, x, starty + MESSAGE_HEIGHT-1, TILE_SIZE);        
    }
    // Call the message module to draw the actual messages.

}
void draw_messages(msg_module_t *message_module){

}
/* image_loader takes a bmp and loads it onto
   a texture to be handed to the renderer.
*/
SDL_Texture *image_loader(const char *filename, SDL_Renderer *renderer){
    SDL_Texture * texture = NULL;
    SDL_Surface *image_surface = IMG_Load(filename);
    if(NULL != image_surface){
        // Attempt to create texture
        texture = SDL_CreateTextureFromSurface(renderer, image_surface);
        if(texture == NULL){
			log_SDL_error(stdout, "CreateTextureFromSurface");
		}
	}
	else {
		printf("IMG_Error: %s\n", IMG_GetError());
	}
	return texture;
}

/* Renders a given texture to the renderer at
   position x and y.
*/
void renderTextureatXY(SDL_Texture *texture, SDL_Renderer *renderer, int x, int y, int size){
    //Setup the destination rectangle to be at the position we want
	SDL_Rect destination;
	destination.x = x*size;
    destination.y = y*size;
    destination.w = size;
    destination.h = size;
	SDL_RenderCopy(renderer, texture, NULL, &destination);
}


/* Function to handle keypresses and
   call appropriate functions. For now
   it simply prints the key pushed to stdout.
*/
void handle_keypress(SDL_Event event){
    if(event.type == SDL_KEYDOWN){
        return;
        //printf("%d - %s\n", event.key.keysym.sym, SDL_GetKeyName(event.key.keysym.sym));
    }
}

void get_input(SDL_Event event){
    handle_keypress(event);
}

/*
    Generic function that checks the current error
    from SDL and sends it to the FILE-pointer given.
    Usually stdotu.
*/
void log_SDL_error(FILE *output, char *message){
    fprintf(output, "%s, Error: %s\n", message, SDL_GetError());
}