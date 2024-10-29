// main.c
#include "all_headers.h"
/*
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "main.h"
#include "dungeon.h"
#include "items.h"
#include "player.h"
#include "map_io.h"
#include "sdl_draw.h"
#include "user_input.h"
#include "sdl_symbols.h"
#include "buttons.h"
#include "monster.h"
#include "messages.h"
*/

// Global variables
time_t start_time;
time_t end_time;

int main(int argc, char *argv[]) {
    //int i;
    //char map_filename[] = "Maps/map1.csv";
    char obj_filename[] = "Maps/objects.csv";
    char buf[150]; // welcome message
    
    chdir("/Users/markpontius/Desktop/Programming/C/Dungeon-Reincarnate-1");
    srand((unsigned int)time(&start_time));
    printf("start_time = %ld\n",start_time);
    sdl_start();                 // initialize graphics library, create window, create timer
    SDL_Surface* icon = IMG_Load("PNG/icon-small.png");
    SDL_SetWindowIcon(window, icon);
    player_init();
    init_battle_arena();
    read_map_csv(1);
    printf("read_map_csv (Map number %d) complete: map is %d,%d\n",map.map_number,map.x_size,map.y_size);
    read_objects_csv(obj_filename);
    printf("read_objects_csv \"%s\" complete: num_objects = %d\n",obj_filename,num_objects);

    load_textures();             // load images for all objects.  
    monster_create(num_monsters); num_monsters++;
    create_all_object_buttons(); // creates a map UI button for each object that needs one

    snprintf(buf,150,"Welcome to Dungeon Reincarnate.  You have arrived at %s",map.map_name);
    printf("%s\n",buf);
    message_create(buf,0);

    main_user_interface_loop();  // MAIN USER INTERFACE LOOP
    
    //print_all_detailed_objects();

    return 0; // Successful execution
}
