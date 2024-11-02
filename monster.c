// player.c
#include "all_headers.h"

/*
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "dungeon.h"
#include "monster.h"
#include "items.h"
#include "player.h"
#include "sdl_draw.h"
#include "sdl_symbols.h"
#include "messages.h"
#include "buttons.h"
*/

// GLOBAL variables
struct monster_s monsters[20];
int num_monsters;

void monster_create(int mnum) {
    monsters[mnum].name = "Mimic";
    monsters[mnum].uid = 1;
    monsters[mnum].common_type = 'M';  // M=for monster
    monsters[mnum].description = "A hungry treasure chest";
    monsters[mnum].graphic_type = 1;   // 1=symbol
    monsters[mnum].graphic_name = "PNG/mimic-chest.png";
    monsters[mnum].graphic_color = color_from_rgba(COLOR_GREEN);
    monsters[mnum].monster_mode = 2;   // 2 = colorized symbol
    monsters[mnum].texture = load_symbol_texture(monsters[mnum].graphic_name, monsters[mnum].graphic_color, monsters[mnum].monster_mode);
    monsters[mnum].frect_count = 0;    // unused
    monsters[mnum].size = 0.75;        // 75% of a tile
    monsters[mnum].valid = 1;          // 1 = Happy
    monsters[mnum].comment = "";       // unused
    monsters[mnum].custom_data = "";   // unused
    
    monsters[mnum].x=10.5;             // current location across map
    monsters[mnum].y=10.5;             // current location down map
    monsters[mnum].dir=3;              // current direction
    monsters[mnum].map_number=1;       // current map
    
    monsters[mnum].route_following=0;  // 0=wandering, 1=following
    monsters[mnum].target_x=0.0;       // Target location for route planning
    monsters[mnum].target_y=0.0;       //
    monsters[mnum].step_size=0.1;      // TODO: Rename speed

    monsters[mnum].health = 5;         // Battle parameters
    monsters[mnum].damage = 2;
    monsters[mnum].weapon_name = "Teeth";
    monsters[mnum].armor_class = 8;

    for (int y=0; y<64; y++) {         // clear the monster's working maps
        for (int x=0; x<64; x++) {
            monsters[mnum].target_map[x][y] = 0;
            monsters[mnum].known_map[x][y] = 0;
        }
    }

    monsters[mnum].option_open_doors=0;  // 1 = will auto-open a door if you try to walk through it.
    monsters[mnum].option_close_doors=0; // 1 = will auto-close a door after you walk through it.
    monsters[mnum].option_avoid_walls=1; // 1 = will walk away from a wall when too close.
    monsters[mnum].option_graphic_dir=0; // 1 = graphic rotates to direction faced.
}

// Teleport, used for initial position mostly
void monster_move_to(int mnum, float x, float y, int dir) {
    monsters[mnum].x = x;
    monsters[mnum].y = y;
    monsters[mnum].dir = dir;
}

// same as player function
float monster_can_move(int mnum, float x, float y, int dir, float distance) {
    float distance_to_wall;
    //float dx, dy;
    if (get_any_wall_xy(x,y,dir)==0x00) {
        //printf("iround[x=%f]=%d, iround[y=%f]=%d\n",monsters[mnum].x, iround(monsters[mnum].x), monsters[mnum].y, iround(monsters[mnum].y));
        //printf("player_can_move(dir=%d, distance=%f): monsters[mnum].x=%f, monsters[mnum].y=%f, distance_to_wall=UNLIMITED, RESULT=%f\n",
                //dir, distance,monsters[mnum].x, monsters[mnum].y, distance);
        return distance;
    } else {
        switch (dir) {
            case 0: distance_to_wall = frac(monsters[mnum].y); break;
            case 1: distance_to_wall = 1.0 - frac(monsters[mnum].x); break;
            case 2: distance_to_wall = 1.0 - frac(monsters[mnum].y); break;
            case 3: distance_to_wall = frac(monsters[mnum].x); break;
            default: exit(-1);
        }
        //printf("player_can_move: dir=%d, monsters[mnum].y=%f, floor(monsters[mnum].y)=%f, ceil(monsters[mnum].y)=%f\n",dir,monsters[mnum].y, floor(monsters[mnum].y), ceil(monsters[mnum].y));
        //printf("player_can_move: dir=%d, monsters[mnum].x=%f, floor(monsters[mnum].x)=%f, ceil(monsters[mnum].x)=%f\n",dir,monsters[mnum].x, floor(monsters[mnum].x), ceil(monsters[mnum].x));
        distance_to_wall -= 2.0*WT;     // subtract off thickness of wall
        if (distance < distance_to_wall) {
            //printf("player_can_move(dir=%d, distance=%f): monsters[mnum].x=%f, monsters[mnum].y=%f, distance_to_wall=%f, RESULT=%f\n",
                    //dir, distance,monsters[mnum].x, monsters[mnum].y, distance_to_wall, distance);
            return (distance);
        } else {
            //printf("player_can_move(dir=%d, distance=%f): monsters[mnum].x=%f, monsters[mnum].y=%f, distance_to_wall=%f, RESULT=%f\n",
                    //dir, distance,monsters[mnum].x, monsters[mnum].y, distance_to_wall, distance_to_wall);
            return (distance_to_wall);
        };
    }
}

void monster_move(int mnum, int dir, float distance) {
    float dx, dy;
    float new_distance;
    dx=0.0; dy=0.0;
    monsters[mnum].dir = dir;
    new_distance = monster_can_move(mnum, monsters[mnum].x, monsters[mnum].y, dir,distance);
    switch (dir) {
        case 0: dy= -new_distance; break;
        case 1: dx=  new_distance; break;
        case 2: dy=  new_distance; break;
        case 3: dx= -new_distance; break;
    }
    monsters[mnum].dir = dir;
    monsters[mnum].x += dx;
    monsters[mnum].y += dy;

    if (monsters[mnum].option_close_doors) {
        if (ifloor(monsters[mnum].x) != ifloor(monsters[mnum].x - dx) && monster_can_open(mnum, monsters[mnum].x, monsters[mnum].y, opposite(dir))) {
            monster_open(mnum,opposite(dir));
        }
        if (ifloor(monsters[mnum].y) != ifloor(monsters[mnum].y - dy) && monster_can_open(mnum, monsters[mnum].x, monsters[mnum].y, opposite(dir))) {
            monster_open(mnum,opposite(dir));
        }
    }
}

// same as player
int monster_can_open(int mnum, float x, float y, int dir) {
    if (get_door(map.walls[ifloor(x)][ifloor(y)],dir)==0x10) {
        return 1;
    } else {
        return 0;
    }
}

int monster_open(int mnum, int dir) {
    float x2, y2;
    if (!monster_can_open(mnum, monsters[mnum].x, monsters[mnum].y, dir)) return(0);
    x2 = monsters[mnum].x;
    y2 = monsters[mnum].y;
    switch(dir) {
        case 0: y2-=1; break;
        case 1: x2+=1; break;
        case 2: y2+=1; break;
        case 3: x2-=1; break;
    }
    // to open/close a door, toggle the wall...
    map.walls[ifloor(monsters[mnum].x)][ifloor(monsters[mnum].y)] ^= (0x01 << dir);
    map.walls[ifloor(x2)][ifloor(y2)] ^= (0x01 << opposite(dir));
    return(1);
}

int monster_follow_route(int mnum) {
    //int dir;
    int current_dist;
    int x = monsters[mnum].x;
    int y = monsters[mnum].y;
    printf("monster_follow_route(%d)\n",mnum);
    if (monsters[mnum].route_following ==0) return 0;     // nothing to follow now
    current_dist = monsters[mnum].target_map[x][y];
    printf("monster route_following (cur_dist %d)\n",current_dist);
    if (current_dist == 0) {
        float x_dist,y_dist;
        // go to exact location in target cell
        x_dist = monsters[mnum].x - monsters[mnum].target_x;
        y_dist = monsters[mnum].y - monsters[mnum].target_y;
        //printf("x_dist(%f) = monsters[mnum].target_x(%f) - monsters[mnum].x(%f)\n",x_dist,monsters[mnum].target_x,monsters[mnum].x);
        //printf("y_dist(%f) = monsters[mnum].target_y(%f) - monsters[mnum].y(%f)\n",y_dist,monsters[mnum].target_y,monsters[mnum].y);
        if        (x_dist>0) { monster_move(mnum,3,(fmin(x_dist,monsters[mnum].step_size))); return 1;
        } else if (x_dist<0) { monster_move(mnum,1,(fmin(-x_dist,monsters[mnum].step_size))); return 1;
        } else if (y_dist>0) { monster_move(mnum,0,(fmin(y_dist,monsters[mnum].step_size))); return 1;
        } else if (y_dist<0) { monster_move(mnum,2,(fmin(-y_dist,monsters[mnum].step_size))); return 1;
        } else {
            monsters[mnum].route_following=0;
            printf("monster arrived at destination (%d)\n",mnum);
            return 0; // arrived at destination
        };
    }
    if (monsters[mnum].option_avoid_walls) {
        char walls = map.walls[ifloor(monsters[mnum].x)][ifloor(monsters[mnum].y)];
        if ((monsters[mnum].dir==0 || monsters[mnum].dir==2) && (frac(monsters[mnum].x)>0.75) && (get_wall(walls,1)==0x01)) {
            monster_move(mnum,3,monsters[mnum].step_size);
            return 1;
        };
        if ((monsters[mnum].dir==0 || monsters[mnum].dir==2) && (frac(monsters[mnum].x)<0.25) && (get_wall(walls,3)==0x01)) {
            monster_move(mnum,1,monsters[mnum].step_size);
            return 1;
        };
        if ((monsters[mnum].dir==1 || monsters[mnum].dir==3) && (frac(monsters[mnum].y)>0.75) && (get_wall(walls,2)==0x01)) {
            monster_move(mnum,0,monsters[mnum].step_size);
            return 1;
        }
        if ((monsters[mnum].dir==1 || monsters[mnum].dir==3) && (frac(monsters[mnum].y)<0.25) && (get_wall(walls,0)==0x01)) {
            monster_move(mnum,2,monsters[mnum].step_size);
            return 1;
        }
    }
    printf("monster looking for move (%d)\n",mnum);
    if ((y>0) && (monsters[mnum].target_map[x][y-1] == current_dist-1) && (player_can_move(monsters[mnum].x, monsters[mnum].y, 0,1.0)==1.0)) {
        monster_move(mnum,0,monsters[mnum].step_size);
        return(1); // Success
    }
    if ((x<map.x_size-1) && (monsters[mnum].target_map[x+1][y] == current_dist-1) && (player_can_move(monsters[mnum].x, monsters[mnum].y, 1,1.0)==1.0)) {
        monster_move(mnum,1,monsters[mnum].step_size);
        return(1); // Success
    }
    if ((y<map.y_size-1) && (monsters[mnum].target_map[x][y+1] == current_dist-1) && (player_can_move(monsters[mnum].x, monsters[mnum].y, 2,1.0)==1.0)) {
        monster_move(mnum,2,monsters[mnum].step_size);
        return(1); // Success
    }
    if ((x>0) && (monsters[mnum].target_map[x-1][y] == current_dist-1) && (player_can_move(monsters[mnum].x, monsters[mnum].y, 3,1.0)==1.0)) {
        monster_move(mnum,3,monsters[mnum].step_size);
        return(1); // Success
    }
    if ((y>0) && (monsters[mnum].target_map[x][y-1] == current_dist-1) && (monsters[mnum].option_open_doors && player_can_open(x,y,0))) {
        monsters[mnum].dir=0;
        monster_open(mnum,0);
        return(2); // Success
    }
    if ((x<map.x_size-1) && (monsters[mnum].target_map[x+1][y] == current_dist-1) && (monsters[mnum].option_open_doors && player_can_open(x,y,1))) {
        monsters[mnum].dir=1;
        monster_open(mnum,1);
        return(2); // Success
    }
    if ((y<map.y_size-1) && (monsters[mnum].target_map[x][y+1] == current_dist-1) && (monsters[mnum].option_open_doors && player_can_open(x,y,2))) {
        monsters[mnum].dir=2;
        monster_open(mnum,2);
        return(2); // Success
    }
    if ((x>0) && (monsters[mnum].target_map[x-1][y] == current_dist-1) && (monsters[mnum].option_open_doors && player_can_open(x,y,3))) {
        monsters[mnum].dir=3;
        monster_open(mnum,3);
        return(2); // Success
    }
    printf("monster failed (%d)\n",mnum);
    return(-1); // Failend
}

int monster_plan_route(int mnum, float route_to_x, float route_to_y, int use_doors, int use_known_space) {
    int x, y;           // current search point
    int ymin, ymax, xmin, xmax; // constrain the seed search area
    unsigned char i;    // distance from original seed
    int grew;           // flag indicating we have grown this iteration
    unsigned char walls; // temporary variable

    monsters[mnum].target_x = route_to_x;
    monsters[mnum].target_y = route_to_y;
    // clear the map
    for (y=0;y<map.y_size;y++) {
        for (x=0;x<map.x_size;x++) {
            monsters[mnum].target_map[x][y] = 0xff;
        }
    }
    // seed the map with a 0 at the target cell
    // do not check for known_space, as we can choose a target beyond.
    ymin = ymax = ifloor(route_to_y); // seed the search range, grows with seed
    xmin = xmax = ifloor(route_to_x);
    monsters[mnum].target_map[xmin][ymin] = 0;   // seed map at target.

    // grow the map up to 50 iterations
    for (i=0;i<100;i++) {
        //printf("begin iteration %d with min=%d,%d and max=%d,%d(grew=%d)\n",i,xmin,ymin,xmax,ymax,grew);
        grew = 0;   // grow until reach current position, or no more growth.
        for (y=(ymin);y<=(ymax);y++) {
            for (x=(xmin);x<=(xmax);x++) {
                if (monsters[mnum].target_map[x][y]==i) {
                    if (x==ifloor(monsters[mnum].x) && y==ifloor(monsters[mnum].y)) {
                        printf("Route found in %d steps\n",i);
                        print_distancemap(monsters[mnum].target_map,xmin,ymin,xmax,ymax);
                        return 1; // SUCCESS
                    }
                    walls = map.walls[x][y];
                    // grow available routes
                    //if (get_wall(walls,0) == 0x00 && y>0 && distance_map[x][y-1]==0xff) {
                    if (y>=0 && y<map.y_size
                                && monsters[mnum].target_map[x][y-1]==0xff
                                && (use_known_space && monsters[mnum].known_map[x][y-1]!=0)
                                && ((get_wall(walls,0) == 0x00)
                                    || (use_doors && (get_door(walls,0) == 0x10)))) {
                        //printf("grow North @ %d,%d\n",x,y);
                        monsters[mnum].target_map[x][y-1] = i+1; ymin=imin(ymin,y-1);
                        grew++;
                    }
                    //if (get_wall(walls,1) == 0x00 && x<map.x_size && distance_map[x+1][y]==0xff) {
                    if (x>=0 && x<map.x_size && y>=0 && y<map.y_size
                                && monsters[mnum].target_map[x+1][y]==0xff
                                && (use_known_space && monsters[mnum].known_map[x+1][y]!=0)
                                && ((get_wall(walls,1) == 0x00)
                                    || (use_doors && (get_door(walls,1) == 0x10)))) {
                        //printf("grow East @ %d,%d\n",x,y);
                        monsters[mnum].target_map[x+1][y] = i+1; xmax=imax(xmax,x+1);
                        grew++;
                    }
                    //if (get_wall(walls,2) == 0x00 && y<map.y_size && distance_map[x][y+1]==0xff) {
                    if (x>=0 && x<map.x_size && y>=0 && y<map.y_size
                                && monsters[mnum].target_map[x][y+1]==0xff
                                && (use_known_space && monsters[mnum].known_map[x][y+1]!=0)
                                && ((get_wall(walls,2) == 0x00)
                                    || (use_doors && (get_door(walls,2) == 0x10)))) {
                        //printf("grow South @ %d,%d\n",x,y);
                        monsters[mnum].target_map[x][y+1] = i+1; ymax=imax(ymax,y+1);
                        grew++;
                    }
                    //if (get_wall(walls,3) == 0x00 && x>0 && distance_map[x-1][y]==0xff) {
                    if (x>=0 && x<map.x_size && y>=0 && y<map.y_size
                                && monsters[mnum].target_map[x-1][y]==0xff
                                && (use_known_space && monsters[mnum].known_map[x-1][y]!=0)
                                && ((get_wall(walls,3) == 0x00)
                                    || (use_doors && (get_door(walls,3) == 0x10)))) {
                        //printf("grow West @ %d,%d\n",x,y);
                        monsters[mnum].target_map[x-1][y] = i+1; xmin=imin(xmin,x-1);
                        grew++;
                    }
                }
            }
        }
        if (grew==0) {
            printf("Route cannot grow after %d steps\n",i);
            print_distancemap(monsters[mnum].target_map,xmin,ymin,xmax,ymax);
            break;
        }
    }
    return 0; // failure
}

// most primitive view algorithm, grows a seed in any direction exposing
// entire room/passages not blocked by walls or closed doors.
int monster_view_simple(int mnum) {
    //result goes into global: unsigned char monsters[mnum].known_map[64][64];
    int x,y,dir;
    unsigned char both_bits;
    int grew;
    int minx,maxx,miny,maxy;
    for (y=0;y<map.y_size;y++) {
        for (x=0;x<map.x_size;x++) {
            if (monsters[mnum].known_map[x][y]==1) monsters[mnum].known_map[x][y]=2;
        };
    };
    // seed view at monster position, ok to be in unknown map area (like behind a door).
    x = ifloor(monsters[mnum].x);
    y = ifloor(monsters[mnum].y);
    monsters[mnum].known_map[x][y]=1;
    minx=x; maxx=x;
    miny=y; maxy=y;
    grew = 1;
    while (grew) {  // loop stops when no more growth
        grew = 0;
        for (y=miny;y<=maxy;y++) {
            for (x=minx;x<=maxx;x++) {
                //printf("view simple %d,%d\n",x,y);
                if (monsters[mnum].known_map[x][y]==1) {
                    for (dir=0;dir<=3;dir++) {
                        both_bits = get_both_bits(map.walls[x][y],dir);
                        if (both_bits == 0x00 || both_bits == 0x10) {
                            switch (dir) {
                            case 0: if(y>0 && monsters[mnum].known_map[x][y-1]!=1) {
                                        monsters[mnum].known_map[x][y-1]=1;
                                        miny=imin(miny,y-1);
                                        grew=1;
                                    }
                                    break;
                            case 1: if(x<map.x_size-1 && monsters[mnum].known_map[x+1][y]!=1) { 
                                        monsters[mnum].known_map[x+1][y]=1;
                                        maxx=imax(maxx,x+1);
                                        grew=1;
                                    }
                                    break;
                            case 2: if(y<map.y_size-1 && monsters[mnum].known_map[x][y+1]!=1) { 
                                        monsters[mnum].known_map[x][y+1]=1;
                                        maxy=imax(maxy,y+1);
                                        grew=1;
                                    }
                                    break;
                            case 3: if(x>0 && monsters[mnum].known_map[x-1][y]!=1) {
                                        monsters[mnum].known_map[x-1][y]=1;
                                        minx=imin(minx,x-1);
                                        grew=1;
                                    }
                                    break;
                            }
                        }
                    }
                }
            }
        }
    }
    //for DEBUG: print the visible map area as text
//    for (y=miny;y<=maxy;y++) {
//        for (x=minx;x<=maxx;x++) {
//            printf("%d",monsters[mnum].known_map[x][y]);
//        };
//        printf("\n");
//    };
    return 1;
}

int monster_visible_to_player(int mnum) {
    int x,y;
    x = ifloor(monsters[mnum].x);
    y = ifloor(monsters[mnum].y);
    return (map.visible[x][y]>=1 && map.visible[x][y]<255);
}

int player_visible_to_monster(int mnum) {
    int x,y;
    x = ifloor(pl.x);
    y = ifloor(pl.y);
    return (monsters[mnum].known_map[x][y]>=1 && monsters[mnum].known_map[x][y]<255);
}

void monster_random_walk(int mnum) {
    int dir;
    monster_view_simple(mnum);
    if (monster_visible_to_player(mnum)) {
        dirty_display = 10;
    }
    if (player_visible_to_monster(mnum)) {
        monster_plan_route(mnum,pl.x,pl.y,0,1);
        monsters[mnum].route_following =1;
        monster_follow_route(mnum);
    } else {
        dir = rand() % 4;
        monster_move(mnum, dir, monsters[mnum].step_size);
    };
    if (monster_visible_to_player(mnum)) {
        dirty_display = 10;
    }
}
