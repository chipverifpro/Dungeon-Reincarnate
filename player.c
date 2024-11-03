// player.c
#include "all_headers.h"

// GLOBAL variables
struct player_s pl;

double dummyd; // necessary for modf, unused

void player_init(void) {
    player_move_to(0.5, 0.5, 1); //overridden by start location coded in map1
    pl.step_size = 1.0/7.0;
    pl.route_following=0;    
    pl.option_open_doors=1;  // will auto-open a door if you try to walk through it
    pl.option_close_doors=1; // will auto-close a door after you walk through it
    pl.option_avoid_walls=1; // will walk away from a wall when too close.
}

void player_move_to(float x, float y, int dir) {
    pl.x = x;
    pl.y = y;
    pl.dir = dir;
}

float player_can_move(float x, float y, int dir, float distance) {
    float distance_to_wall;
    //float dx, dy;
    if (get_any_wall_xy(x,y,dir)==0x00) {
        //printf("iround[x=%f]=%d, iround[y=%f]=%d\n",pl.x, iround(pl.x), pl.y, iround(pl.y));
        //printf("player_can_move(dir=%d, distance=%f): pl.x=%f, pl.y=%f, distance_to_wall=UNLIMITED, RESULT=%f\n",
                //dir, distance,pl.x, pl.y, distance);
        return distance;
    } else {
        switch (dir) {
            case 0: distance_to_wall = frac(pl.y); break;
            case 1: distance_to_wall = 1.0 - frac(pl.x); break;
            case 2: distance_to_wall = 1.0 - frac(pl.y); break;
            case 3: distance_to_wall = frac(pl.x); break;
            default: exit(-1);
        }
        //printf("player_can_move: dir=%d, pl.y=%f, floor(pl.y)=%f, ceil(pl.y)=%f\n",dir,pl.y, floor(pl.y), ceil(pl.y));
        //printf("player_can_move: dir=%d, pl.x=%f, floor(pl.x)=%f, ceil(pl.x)=%f\n",dir,pl.x, floor(pl.x), ceil(pl.x));
        distance_to_wall -= 2.0*WT;     // subtract off thickness of wall
        if (distance < distance_to_wall) {
            //printf("player_can_move(dir=%d, distance=%f): pl.x=%f, pl.y=%f, distance_to_wall=%f, RESULT=%f\n",
                    //dir, distance,pl.x, pl.y, distance_to_wall, distance);
            return (distance);
        } else {
            //printf("player_can_move(dir=%d, distance=%f): pl.x=%f, pl.y=%f, distance_to_wall=%f, RESULT=%f\n",
                    //dir, distance,pl.x, pl.y, distance_to_wall, distance_to_wall);
            return (distance_to_wall);
        };
    }
}

void player_move(int dir, float distance) {
    float dx, dy;
    float new_distance;
    dx=0.0; dy=0.0;
    pl.dir = dir;
    new_distance = player_can_move(pl.x, pl.y, dir,distance);
    switch (dir) {
        case 0: dy= -new_distance; break;
        case 1: dx=  new_distance; break;
        case 2: dy=  new_distance; break;
        case 3: dx= -new_distance; break;
    }
    pl.dir = dir;
    pl.x += dx;
    pl.y += dy;

    if (pl.option_close_doors) {
        if (ifloor(pl.x) != ifloor(pl.x - dx) && player_can_open(pl.x, pl.y, opposite(dir))) {
            player_open(opposite(dir));
        }
        if (ifloor(pl.y) != ifloor(pl.y - dy) && player_can_open(pl.x, pl.y, opposite(dir))) {
            player_open(opposite(dir));
        }
    }
}

int player_can_open(float x, float y, int dir) {
    if (get_door(map.walls[ifloor(x)][ifloor(y)],dir)==0x10) {
        return 1;
    } else {
        return 0;
    }
}

int player_open(int dir) {
    float x2, y2;
    if (!player_can_open(pl.x, pl.y, dir)) return(0);
    x2 = pl.x;
    y2 = pl.y;
    switch(dir) {
        case 0: y2-=1; break;
        case 1: x2+=1; break;
        case 2: y2+=1; break;
        case 3: x2-=1; break;
    }
    // to open/close a door, toggle the wall...
    map.walls[ifloor(pl.x)][ifloor(pl.y)] ^= (0x01 << dir);
    map.walls[ifloor(x2)][ifloor(y2)] ^= (0x01 << opposite(dir));
    return(1);
}

// calculate distance as squareroot of squares
float get_distance(float x1,float y1, float x2,float y2) {
    return (sqrt(pow(x1-x2,2)+pow(y1-y2,2)));
}

int imin(int a, int b) {
    if (a<=b) return a;
    return b;
}
int imax(int a, int b) {
    if (a>=b) return a;
    return b;
}

int player_follow_route(void) {
    //int dir;
    float step_size;
    int current_dist;
    int x = ifloor(pl.x);
    int y = ifloor(pl.y);
    if (pl.route_following ==0) return 0;     // nothing to follow now
    step_size = pl.step_size * map.travel_distance * map.terrains[get_terrain_xy(pl.x,pl.y)].travel_cost / 10000.0;
    //printf("pl.step_size = %f, map.travel_distance = %d/100, terrain.travel_cosst = %5d/100\n",pl.step_size,map.travel_distance,map.terrains[get_terrain_xy(pl.x,pl.y)].travel_cost);
    current_dist = pl.target_map[ifloor(x)][ifloor(y)];
    if (current_dist == 0) {
        float x_dist,y_dist;
        // go to exact location in target cell
        x_dist = pl.x - pl.target_x;
        y_dist = pl.y - pl.target_y;
        //printf("x_dist(%f) = pl.target_x(%f) - pl.x(%f)\n",x_dist,pl.target_x,pl.x);
        //printf("y_dist(%f) = pl.target_y(%f) - pl.y(%f)\n",y_dist,pl.target_y,pl.y);
        if        (x_dist>0) { player_move(3,(fmin(x_dist, step_size))); return 1;
        } else if (x_dist<0) { player_move(1,(fmin(-x_dist,step_size))); return 1;
        } else if (y_dist>0) { player_move(0,(fmin(y_dist, step_size))); return 1;
        } else if (y_dist<0) { player_move(2,(fmin(-y_dist,step_size))); return 1;
        } else {
            pl.route_following=0;
            return 0; // arrived at destination
        };
    }
    if (pl.option_avoid_walls) {
        if ((pl.dir==0 || pl.dir==2) && (frac(pl.x)>0.75) && (get_wall_xy(pl.x,pl.y,1)==0x01)) {
            player_move(3,step_size);
            return 1;
        };
        if ((pl.dir==0 || pl.dir==2) && (frac(pl.x)<0.25) && (get_wall_xy(pl.x,pl.y,3)==0x01)) {
            player_move(1,step_size);
            return 1;
        };
        if ((pl.dir==1 || pl.dir==3) && (frac(pl.y)>0.75) && (get_wall_xy(pl.x,pl.y,2)==0x01)) {
            player_move(0,step_size);
            return 1;
        }
        if ((pl.dir==1 || pl.dir==3) && (frac(pl.y)<0.25) && (get_wall_xy(pl.x,pl.y,0)==0x01)) {
            player_move(2,step_size);
            return 1;
        }
    }
    if ((y>0) && (pl.target_map[x][y-1] == current_dist-1) && (player_can_move(pl.x, pl.y, 0,1.0)==1.0)) {
        player_move(0,step_size);
        return(1); // Success
    }
    if ((x<map.x_size-1) && (pl.target_map[x+1][y] == current_dist-1) && (player_can_move(pl.x, pl.y, 1,1.0)==1.0)) {
        player_move(1,step_size);
        return(1); // Success
    }
    if ((y<map.y_size-1) && (pl.target_map[x][y+1] == current_dist-1) && (player_can_move(pl.x, pl.y, 2,1.0)==1.0)) {
        player_move(2,step_size);
        return(1); // Success
    }
    if ((x>0) && (pl.target_map[x-1][y] == current_dist-1) && (player_can_move(pl.x, pl.y, 3,1.0)==1.0)) {
        player_move(3,step_size);
        return(1); // Success
    }
    if ((y>0) && (pl.target_map[x][y-1] == current_dist-1) && (pl.option_open_doors && player_can_open(x,y,0))) {
        pl.dir=0;
        player_open(0);
        return(2); // Success
    }
    if ((x<map.x_size-1) && (pl.target_map[x+1][y] == current_dist-1) && (pl.option_open_doors && player_can_open(x,y,1))) {
        pl.dir=1;
        player_open(1);
        return(2); // Success
    }
    if ((y<map.y_size-1) && (pl.target_map[x][y+1] == current_dist-1) && (pl.option_open_doors && player_can_open(x,y,2))) {
        pl.dir=2;
        player_open(2);
        return(2); // Success
    }
    if ((x>0) && (pl.target_map[x-1][y] == current_dist-1) && (pl.option_open_doors && player_can_open(x,y,3))) {
        pl.dir=3;
        player_open(3);
        return(2); // Success
    }
    return(-1); // Failed
}


//unsigned char distance_map[68][68];

void print_distancemap(unsigned char distance_map[68][68], int minx,int miny,int maxx,int maxy) {
    int x,y;
    for (y=miny;y<=maxy;y++) {
        for (x=minx;x<=maxx;x++) {
            printf(" %02x",distance_map[x][y]);
        }
        printf("\n");
    }
}

int plan_route(unsigned char distance_map[68][68], float route_to_x, float route_to_y, int use_doors, int use_known_space) {
    int x, y;           // current search point
    int ymin, ymax, xmin, xmax; // constrain the seed search area
    unsigned char i;    // distance from original seed
    int grew;           // flag indicating we have grown this iteration
    unsigned int walls; // temporary variable
    
    pl.target_x = route_to_x;
    pl.target_y = route_to_y;
    // clear the map
    for (y=0;y<map.y_size;y++) {
        for (x=0;x<map.x_size;x++) {
            distance_map[x][y] = 0xff;
        }
    }
    // seed the map with a 0 at the target cell
    // do not check for known_space, as we can choose a target beyond.
    ymin = ymax = ifloor(route_to_y); // seed the search range, grows with seed
    xmin = xmax = ifloor(route_to_x);
    if ((xmin < 0) || (ymin < 0) | (xmax >= map.x_size) || (ymax >= map.y_size)){
        return 0;  // FAILURE: cannot seed outside map limits.
    }
    distance_map[xmin][ymin] = 0;   // seed map at target.
    use_known_space = 0;
    // grow the map up to 50 iterations
    for (i=0;i<100;i++) {
        //printf("begin iteration %d with min=%d,%d and max=%d,%d(grew=%d)\n",i,xmin,ymin,xmax,ymax,grew);
        grew = 0;   // grow until reach current position, or no more growth.
        if (i==1) use_known_space=1;
        for (y=(ymin);y<=(ymax);y++) {
            for (x=(xmin);x<=(xmax);x++) {
                if (distance_map[x][y]==i) {
                    if (x==ifloor(pl.x) && y==ifloor(pl.y)) {
                        printf("Route found in %d steps\n",i);
                        print_distancemap(distance_map,xmin,ymin,xmax,ymax);
                        return 1; // SUCCESS
                    }
                    walls = map.walls[x][y];
                    // grow available routes
                    //if (get_wall(walls,0) == 0x00 && y>0 && distance_map[x][y-1]==0xff) {
                    //if (x>=0 && x<map.x_size && y>=0 && y<map.y_size
                    if (y>=1
                                && distance_map[x][y-1]==0xff
                                && (!use_known_space || map.visible[x][y-1]!=0)
                                && ((get_any_wall_xy(x,y,0) == 0x00)
                                    || (use_doors && (get_door(walls,0) == 0x10)))) {
                        //printf("grow North @ %d,%d\n",x,y);
                        distance_map[x][y-1] = i+1; ymin=imin(ymin,y-1);
                        grew++;
                    }
                    //if (get_wall(walls,1) == 0x00 && x<map.x_size && distance_map[x+1][y]==0xff) {
                    //if (x>=0 && x<map.x_size && y>=0 && y<map.y_size
                    if (x<map.x_size-1
                                && distance_map[x+1][y]==0xff
                                && (!use_known_space || map.visible[x+1][y]!=0)
                                && ((get_any_wall_xy(x,y,1) == 0x00)
                                    || (use_doors && (get_door(walls,1) == 0x10)))) {
                        //printf("grow East @ %d,%d\n",x,y);
                        distance_map[x+1][y] = i+1; xmax=imax(xmax,x+1);
                        grew++;
                    }
                    //if (get_wall(walls,2) == 0x00 && y<map.y_size && distance_map[x][y+1]==0xff) {
                    //if (x>=0 && x<map.x_size && y>=0 && y<map.y_size
                    if (y<map.y_size-1
                                && distance_map[x][y+1]==0xff
                                && (!use_known_space || map.visible[x][y+1]!=0)
                                && ((get_any_wall_xy(x,y,2) == 0x00)
                                    || (use_doors && (get_door(walls,2) == 0x10)))) {
                        //printf("grow South @ %d,%d\n",x,y);
                        distance_map[x][y+1] = i+1; ymax=imax(ymax,y+1);
                        grew++;
                    }
                    //if (get_wall(walls,3) == 0x00 && x>0 && distance_map[x-1][y]==0xff) {
                    //if (x>=0 && x<map.x_size && y>=0 && y<map.y_size
                    if (x>=1
                                && distance_map[x-1][y]==0xff
                                && (!use_known_space || map.visible[x-1][y]!=0)
                                && ((get_any_wall_xy(x,y,3) == 0x00)
                                    || (use_doors && (get_door(walls,3) == 0x10)))) {
                        //printf("grow West @ %d,%d\n",x,y);
                        distance_map[x-1][y] = i+1; xmin=imin(xmin,x-1);
                        grew++;
                    }
                }
            }
        }
        if (grew==0) {
            printf("Route cannot grow after %d steps\n",i);
            print_distancemap(distance_map,xmin,ymin,xmax,ymax);
            break;
        }
    }
    return 0; // failure
}
