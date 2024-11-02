// dungeon.c
#include "all_headers.h"

// GLOBAL variables

// defined in dungeon.h
struct map_s map;
struct map_s maps[10];
int valid_maps[10];
struct object_s objects[50];
int num_objects;
int current_map_num;    // assigned on load

//get whether wall present in dir
int get_wall(int number, int dir) {
    return (number >> dir) & 0x01;
}

int get_wall_xy(float x, float y, int dir) {
    return ((map.walls[ifloor(x)][ifloor(y)]>>dir) & 0x01);
}

//get whether door present in dir
int get_door(int number, int dir) {
    return (number >> dir) & 0x10;
}

int get_door_xy(float x, float y, int dir) {
    return ((map.walls[ifloor(x)][ifloor(y)]>>dir) & 0x10);
}

// invisible wall in dir
int get_keepout(int number, int dir) {
    return (number >> dir) & 0x100;
}

int get_keepout_xy(float x, float y, int dir) {
    return ((map.walls[ifloor(x)][ifloor(y)]>>dir) & 0x100);
}

//get whether door and wall present in dir (two hex nibbles)
int get_both_bits(int number, int dir) {
    return (number >> dir) & 0x11;
}

int get_both_bits_xy(float x, float y, int dir) {
    return ((map.walls[ifloor(x)][ifloor(y)]>>dir) & 0x11);
}

// Toggles wall, and sets opposite side to match.
// using offset, you can toggle walls(0), doors(4), and invisible walls (8).
int toggle_wall_xy(float x, float y, int dir, int offset) {
    int origbit;
    //number = map.walls[ifloor(x)][ifloor(y)];
    origbit = (map.walls[ifloor(x)][ifloor(y)] >> (dir + offset)) & 0x01;
    map.walls[ifloor(x)][ifloor(y)] ^= (1 << (dir + offset));
    // now toggle opposite side of wall, only if it matches polarity
    switch (dir) {
        case 0:  y--; break;
        case 1:  x++; break;
        case 2:  y++; break;
        case 3:  x--; break;
    }
    dir = opposite(dir);
    if ((map.walls[ifloor(x)][ifloor(y)] >> (dir + offset) & 0x01) == origbit) {
        map.walls[ifloor(x)][ifloor(y)] ^= (1 << (dir + offset));
        printf("Toggling opposite side\n");
    } else {
        printf("Not toggling opposite side\n");
    }
    return (1-origbit);
}

//int get_any_wall(int number, int dir) {
//    return ((number >> dir) | (number >> (dir+8))) & 0x01;
//}

int get_any_wall_xy(float x, float y, int dir) {
    uint32_t number;
    int result;
    number = map.walls[ifloor(x)][ifloor(y)];
    result = ((number >> dir) | (number >> (dir+8))) & 0x01;
    // now check opposite side of wall for half-barrier
    switch (dir) {
        case 0:  
            if (ifloor(y)<=0) {
                number = 0xf;
            } else {
                number = map.walls[ifloor(x)][ifloor(y-1)];
            }
            break;
        case 1:  
            if (x>=map.x_size-1) {
                number = 0xf;
            } else {
                number = map.walls[ifloor(x+1)][ifloor(y)];
            }
            break;
        case 2:
            if (y>=map.y_size-1) {
                number = 0xf;
            } else {
                number = map.walls[ifloor(x)][ifloor(y+1)];
            }
            break;
        case 3:
            if (ifloor(x)<=0) {
                number = 0xf;
            }else {
                number = map.walls[ifloor(x-1)][ifloor(y)];
            }
            break;
    }
    dir = opposite(dir);
    result |= ((number >> dir) | (number >> (dir+8))) & 0x01;
    return (result);
}

//get the opposite of dir (0->2, 1->3, 2->0, 3->1)
int opposite(int dir) {
    return ((dir +2)%4);
}

//get the right rotation of dir (0->1, 1->2, 2->3, 3->0)
int rotate_r(int dir) {
    return ((dir +1)%4);
}

//get the left rotation of dir (0->3, 1->0, 2->1, 3->2)
int rotate_l(int dir) {
    return ((dir +3)%4);
}

//get the angle for dir in degrees
float dir_to_degrees (int dir) {
    return (90.0 * dir);
}

// most primitive view algorithm, grows a seed in any direction exposing
// entire room/passages not blocked by walls or closed doors.
int player_view_simple(void) {
    //result goes into global: unsigned char map.visible[64][64];
    int x,y,dir;
    unsigned char both_bits;
    int grew;
    int minx,maxx,miny,maxy;
    for (y=0;y<map.y_size;y++) {
        for (x=0;x<map.x_size;x++) {
            if (map.visible[x][y]==1) map.visible[x][y]=2;
        };
    };
    // seed view at player position, ok to be in unknown map area (like behind a door).
    x = ifloor(pl.x);
    y = ifloor(pl.y);
    map.visible[x][y]=1;
    minx=x; maxx=x;
    miny=y; maxy=y;
    grew = 1;
    while (grew) {  // loop stops when no more growth
        grew = 0;
        for (y=miny;y<=maxy;y++) {
            for (x=minx;x<=maxx;x++) {
                //printf("view simple %d,%d\n",x,y);
                if (map.visible[x][y]==1) {
                    for (dir=0;dir<=3;dir++) {
                        both_bits = get_both_bits(map.walls[x][y],dir);
                        if (both_bits == 0x00 || both_bits == 0x10) {
                            switch (dir) {
                            case 0: if(y>0 && map.visible[x][y-1]!=1) {
                                        map.visible[x][y-1]=1;
                                        miny=imin(miny,y-1);
                                        grew=1;
                                    }
                                    break;
                            case 1: if(x<map.x_size-1 && map.visible[x+1][y]!=1) { 
                                        map.visible[x+1][y]=1;
                                        maxx=imax(maxx,x+1);
                                        grew=1;
                                    }
                                    break;
                            case 2: if(y<map.y_size-1 && map.visible[x][y+1]!=1) { 
                                        map.visible[x][y+1]=1;
                                        maxy=imax(maxy,y+1);
                                        grew=1;
                                    }
                                    break;
                            case 3: if(x>0 && map.visible[x-1][y]!=1) {
                                        map.visible[x-1][y]=1;
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
    return 1;
}

int get_terrain_xy(int x, int y) {
    return((map.walls[x][y] >> 12) & 0xF);
}

int get_view_cost_xy(int x, int y) {
    return (map.terrains[get_terrain_xy(x,y)].view_cost);     // Get from terrain
}

int get_travel_cost_xy(int x, int y) {
    return (map.terrains[get_terrain_xy(x,y)].travel_cost);   // Get from terrain
}

int get_view_distance(void) {
    return (map.view_distance);     // Get from map
}

int get_travel_distance(void) {
    return (map.travel_distance);     // Get from map
}

// version of simple view, but cannot see around corners very well
int player_view_noback(void) {
    //result goes into global: unsigned char map.visible[64][64];
    int x,y;
    int dir;
    int iplx, iply;
    unsigned char both_bits;
    int grew;
    int minx,maxx,miny,maxy;
    int view_cost;
    int max_view;
    int view_budget;
    max_view=get_view_distance();
    for (y=0;y<map.y_size;y++) {
        for (x=0;x<map.x_size;x++) {
            if (map.visible[x][y]>0) map.visible[x][y]=255;
        };
    };
    // seed view at player position.
    iplx = ifloor(pl.x);
    iply = ifloor(pl.y);
    minx=iplx; maxx=iplx;
    miny=iply; maxy=iply;
    // seed view with maximum view distance aka budget
    map.visible[iplx][iply]=get_view_distance();
    grew = 1;
    while (grew) {  // loop stops when no more growth
        grew = 0;
        for (y=miny;y<=maxy;y++) {
            for (x=minx;x<=maxx;x++) {
                view_budget = map.visible[x][y];
                view_cost = get_view_cost_xy(x,y);
                if (view_budget <=250 && view_budget >0) {
                    view_budget -= view_cost;
                }
                if (map.visible[x][y]>=1 && map.visible[x][y]<250 && view_budget>0) {
                    //printf("view noback %d,%d. view_budget=%d, view_cost=%d\n",x,y,view_budget, view_cost);
                    for (dir=0;dir<=3;dir++) {
                        both_bits = get_both_bits_xy(x,y,dir);
                        if (dir==0 && y>iply) both_bits=-1;
                        if (dir==1 && x<iplx) both_bits=-1;
                        if (dir==2 && y<iply) both_bits=-1;
                        if (dir==3 && x>iplx) both_bits=-1;
                        if (both_bits == 0x00 || both_bits == 0x10) {
                            switch (dir) {
                            case 0: if(y>0 && map.visible[x][y-1]!=1) {
                                        map.visible[x][y-1]=view_budget;
                                        map.visible[x][y]=254;
                                        miny=imin(miny,y-1);
                                        grew=1;
                                    }
                                    break;
                            case 1: if(x<map.x_size-1 && map.visible[x+1][y]!=1) { 
                                        map.visible[x+1][y]=view_budget;
                                        map.visible[x][y]=254;
                                        maxx=imax(maxx,x+1);
                                        grew=1;
                                    }
                                    break;
                            case 2: if(y<map.y_size-1 && map.visible[x][y+1]!=1) { 
                                        map.visible[x][y+1]=view_budget;
                                        map.visible[x][y]=254;
                                        maxy=imax(maxy,y+1);
                                        grew=1;
                                    }
                                    break;
                            case 3: if(x>0 && map.visible[x-1][y]!=1) {
                                        map.visible[x-1][y]=view_budget;
                                        map.visible[x][y]=254;
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
// .  for DEBUG: print the visible map area as text
//    for (y=miny;y<=maxy;y++) {
//        for (x=minx;x<=maxx;x++) {
//            printf("%d",map.visible[x][y]);
//        };
//        printf("\n");
//    };
    return 1;
}
