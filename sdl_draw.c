// sdl_gui.c
#include "all_headers.h"

//globals
SDL_Window *window;       // SDL context
SDL_Renderer *renderer;   // SDL context
//SDL_Texture *walls_texture[10]; // for custom walls
//int num_walls_texture;
SDL_Texture *texture;     // NOT the screen, but a separate drawing target.
int timer_id;             // don't know what this may be needed for
int option_enable_shadows = 0;

// globals with external clients
int dirty_display = 1;             // when 0, no redraw is necessary.
float map_origin_x, map_origin_y;  // in floating point screen coordinates
float sw_scale = SW_SCALE;         // scales the whole map (allows zoom)
int screen_width = SCREEN_WIDTH;   // from window manager
int screen_height = SCREEN_HEIGHT; // from window manager

// symbols drawn using lines between a list of points instead of bitmaps
struct F_Point_s player_symbol_points[] = { // player arrow
                    {.65,.4},
                    {.5,.25},
                    {.35,.4},
                    {.5,.25},
                    {.5,.75} };
int       player_symbol_pcount = 5;

// wall sections as lists of rectangles
struct F_Rect_s wall_solid[] = {
                    {0,0, 1.0,WT} };
int      wall_solid_rcount = 1;

struct F_Rect_s door_closed[]= {
                    {0,0,      .3,WT},
                    {.3-WT,WT, WT,.15},
                    {.3,.15,   .4,WT},
                    {.7,WT,    WT,.15},
                    {.7,0,     .3,WT} };
int      door_closed_rcount = 5;
                          
struct F_Rect_s door_open[]  = {
                    {0,0,      .3,WT},
                    {.3-WT,WT, WT,.15},
                    {.7,WT,    WT,.15},
                    {.7,0,     .3,WT} };
int      door_open_rcount = 4;

// Features as list of rectangles
struct F_Rect_s cell_floor[] = {
                    {0,0,      1.0,1.0} };
int      cell_floor_rcount = 1;

struct F_Rect_s stairs_up[] = {
                    {.2, .1,   .6,.05},
                    {.15,.25,  .7,.05},
                    {.1, .4,   .8,.05},
                    {.1, .55,  .8,.05},
                    {.1, .7,   .8,.05},
                    {.1, .85,  .8,.05} };
int      stairs_up_rcount = 6;

struct F_Rect_s stairs_down[] = {
                    {.1, .85,  .8,.05},
                    {.15,.7,   .7,.05},
                    {.2, .55,  .6,.05},
                    {.25,.4,   .5,.05},
                    {.3 ,.25,  .4,.05},
                    {.35,.1,   .3,.05} };
int      stairs_down_rcount = 6;

// functions to move, scale, and rotate lists of rects or points:
//   must call in order as is done here or as in sdl_draw_transformed_rects()

// BEFORE call get_frect_by_name
// FIRST  call is ROTATE and/or SCALE_TO_CENTER
// SECOND call is SCALE (by sw_scale amount)
// THIRD  call is MOVE (to the screen tile)
// LAST   call is CONVERT (to inegers)

// ---------------------------------------------
// looks up pseudo symbols (F_Recct_s or F_Point_s) by name, also returns rcount and graphic type
void *get_frect_by_name(char *frect_name, int *count, int *graphic_type) {
    if (strcmp(frect_name, "stairs_up") ==0) {
        *graphic_type = 2;
        *count = stairs_up_rcount;
        printf("stairs_up matched frect\n");
        return((void *) stairs_up);
    } else if (strcmp(frect_name, "stairs_down") ==0) {
        *graphic_type = 2;
        *count = stairs_down_rcount;
        printf("stairs_down matched frect\n");
        return((void *) stairs_down);
    }
    return (NULL);
}

// Rotates F_Rect list to dir.
struct F_Rect_s *rotate_rects(struct F_Rect_s *src, int num, int dir) {
    int a;
    struct F_Rect_s *dest;
    if (num==0 || src==NULL) return NULL;
    dest = malloc(num * sizeof(struct F_Rect_s));
    if (dest==NULL) {printf("malloc failure\n");exit(-1);}
    for (a=0; a<num; a++) {
        switch (dir) {
        case 0: // North
            dest[a] = src[a];
            break;
        case 1: // East
            dest[a].x = 1.0 - src[a].y - src[a].h;
            dest[a].y = src[a].x;
            dest[a].w = src[a].h;
            dest[a].h = src[a].w;
            break;
        case 2: // South
            dest[a].x = 1.0 - src[a].x - src[a].w;
            dest[a].y = 1.0 - src[a].y - src[a].h;
            dest[a].w = src[a].w;
            dest[a].h = src[a].h;
            break;
        case 3: // West
            dest[a].x = src[a].y;
            dest[a].y = 1.0 - src[a].x - src[a].w;
            dest[a].w = src[a].h;
            dest[a].h = src[a].w;
            break;
        }
    }
    return (dest);
}

// scales F_Rect list around center of tile
struct F_Rect_s *scale_to_center_rects(struct F_Rect_s *src, int num, float sx, float sy) {
    int a;
    struct F_Rect_s *dest;
    if (num==0 || src==NULL) return NULL;
    dest = malloc(num * sizeof(struct F_Rect_s));
    if (dest==NULL) {printf("malloc failure\n");exit(-1);}
    for (a=0; a<num; a++) {
        dest[a].x = (src[a].x-0.5) * sx + 0.5;
        dest[a].y = (src[a].y-0.5) * sy + 0.5;
        dest[a].w = src[a].w * sx;
        dest[a].h = src[a].h * sy;
    }
    return (dest);
}

// scales F-Rect list by (usually) sw_scale, origin is upper left corner
struct F_Rect_s *scale_rects(struct F_Rect_s *src, int num, float sx, float sy) {
    int a;
    struct F_Rect_s *dest;
    if (num==0 || src==NULL) return NULL;
    dest = malloc(num * sizeof(struct F_Rect_s));
    if (dest==NULL) {printf("malloc failure\n");exit(-1);}
    for (a=0; a<num; a++) {
        dest[a].x = src[a].x * sx;
        dest[a].y = src[a].y * sy;
        dest[a].w = src[a].w * sx;
        dest[a].h = src[a].h * sy;
    }
    return (dest);
}

// moves F_Rect list to a tile screen location.
struct F_Rect_s *move_rects(struct F_Rect_s *src, int num, int dx, int dy) {
    int a;
    struct F_Rect_s *dest;
    if (num==0 || src==NULL) return NULL;
    dest = malloc(num * sizeof(struct F_Rect_s));
    if (dest==NULL) {printf("malloc failure\n");exit(-1);}
    for (a=0; a<num; a++) {
        dest[a].x = src[a].x + dx;
        dest[a].y = src[a].y + dy;
        dest[a].w = src[a].w;
        dest[a].h = src[a].h;
    }
    return (dest);
}

// Converts F_Rect (float) list to SDL_Rect (integer) list for drawing.
SDL_Rect *convert_rects(struct F_Rect_s *src, int num) {
    int a;
    SDL_Rect *dest;
    if (num==0 || src==NULL) return NULL;
    dest = malloc(num * sizeof(SDL_Rect));
    if (dest==NULL) {printf("malloc failure\n");exit(-1);}
    for (a=0; a<num; a++) {
        dest[a].x = iround(src[a].x);
        dest[a].y = iround(src[a].y);
        dest[a].w = iround(src[a].w);
        dest[a].h = iround(src[a].h);
    }
    return (dest);
}

// takes an F_Rect list with dir, location, and size and draws it.
void sdl_draw_transformed_rects(struct F_Rect_s *rects, int rcount, int dir, float tile_x, float tile_y, float size_x, float size_y){
    struct F_Rect_s *temp0, *temp1, *temp2, *temp3;  // temporary list of rectangles
    SDL_Rect *scr_rects;
    if (rects==NULL) return;
    temp0 = scale_to_center_rects(rects,rcount,size_x,size_y); // enlarge to eliminate rounding errors causing gaps.
    temp1 = rotate_rects(temp0,rcount, dir);            
    temp2 = scale_rects(temp1,rcount, sw_scale+.01, sw_scale+.01); // .01 gets rid of gaps due to rounding errors.
    temp3 = move_rects(temp2,rcount, tile_x, tile_y);            
    scr_rects = convert_rects(temp3,rcount);
    if (scr_rects != NULL){
        //SDL_RenderDrawRects(renderer, scr_rects, rcount); //Hollow Walls
        SDL_RenderFillRects(renderer, scr_rects, rcount); //Solid Walls with WT>0
        free(temp0); free(temp1); free(temp2); free(temp3);
        free(scr_rects);
    }
}

// ---------------------------------------------
// returns F_Point list rotated to direction DIR
struct F_Point_s *rotate_points(struct F_Point_s *src, int num, int dir) {
    int a;
    struct F_Point_s *dest;
    if (num==0 || src==NULL) return NULL;
    dest = malloc(num * sizeof(struct F_Point_s));
    if (dest==NULL) {printf("malloc failure\n");exit(-1);}
    for (a=0; a<num; a++) {
        switch (dir) {
        case 0: // North
            dest[a] = src[a];
            break;
        case 1: // East
            dest[a].x = 1.0 - src[a].y;
            dest[a].y = src[a].x;
            break;
        case 2: // South
            dest[a].x = 1.0 - src[a].x;
            dest[a].y = 1.0 - src[a].y;
            break;
        case 3: // West
            dest[a].x = src[a].y;
            dest[a].y = 1.0 - src[a].x;
            break;
        }
    }
    return (dest);
}

struct F_Point_s *scale_to_center_points(struct F_Point_s *src, int num, float sx, float sy) {
    int a;
    struct F_Point_s *dest;
    if (num==0 || src==NULL) return NULL;
    dest = malloc(num * sizeof(struct F_Point_s));
    if (dest==NULL) {printf("malloc failure\n");exit(-1);}
    for (a=0; a<num; a++) {
        dest[a].x = (src[a].x-0.5) * sx + 0.5;
        dest[a].y = (src[a].y-0.5) * sy + 0.5;
    }
    return (dest);
}

struct F_Point_s *scale_points(struct F_Point_s *src, int num, float sx, float sy) {
    int a;
    struct F_Point_s *dest;
    if (num==0 || src==NULL) return NULL;
    dest = malloc(num * sizeof(struct F_Point_s));
    if (dest==NULL) {printf("malloc failure\n");exit(-1);}
    for (a=0; a<num; a++) {
        dest[a].x = src[a].x * sx;
        dest[a].y = src[a].y * sy;
    }
    return (dest);
}

struct F_Point_s *move_points(struct F_Point_s *src, int num, int dx, int dy) {
    int a;
    struct F_Point_s *dest;
    if (num==0 || src==NULL) return NULL;
    dest = malloc(num * sizeof(struct F_Point_s));
    if (dest==NULL) {printf("malloc failure\n");exit(-1);}
    for (a=0; a<num; a++) {
        dest[a].x = src[a].x + dx;
        dest[a].y = src[a].y + dy;
    }
    return (dest);
}

SDL_Point *convert_points(struct F_Point_s *src, int num) {
    int a;
    SDL_Point *dest;
    if (num==0 || src==NULL) return NULL;
    dest = malloc(num * sizeof(SDL_Point));
    if (dest==NULL) {printf("malloc failure\n");exit(-1);}
    for (a=0; a<num; a++) {
        dest[a].x = iround(src[a].x);
        dest[a].y = iround(src[a].y);
    }
    return (dest);
}

// takes an F_Point list with dir, location, and size and draws it.
void sdl_draw_transformed_lines(struct F_Point_s *symbol_points, int pcount, int dir, float tile_x, float tile_y, float size_x, float size_y){
    // Draw Player
    struct F_Point_s *ptemp0, *ptemp1, *ptemp2, *ptemp3;
    SDL_Point *scr_points;
    ptemp0 = scale_to_center_points(symbol_points,pcount, size_x,size_y);
    ptemp1 = rotate_points(ptemp0,pcount, dir);
    ptemp2 = scale_points(ptemp1,pcount, sw_scale, sw_scale);
    ptemp3 = move_points(ptemp2,pcount, tile_x, tile_y);
    scr_points = convert_points(ptemp3,pcount);
    if (scr_points != NULL){
        SDL_RenderDrawLines(renderer,scr_points,pcount);
        free(ptemp0); free(ptemp1); free(ptemp2); free(ptemp3);
        free(scr_points);
    }
}

SDL_Vertex *convert_scr_points_to_vertexes(SDL_Point *src, int num,
                    uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha) {
    int a;
    SDL_Vertex *dest;
    if (num==0 || src==NULL) return NULL;
    dest = malloc(num * sizeof(SDL_Vertex));
    if (dest==NULL) {printf("malloc failure\n");exit(-1);}
    for (a=0; a<num; a++) {
        dest[a].position.x = src[a].x;
        dest[a].position.y = src[a].y;
        dest[a].color.r = red;
        dest[a].color.g = green;
        dest[a].color.b = blue;
        dest[a].color.a = alpha;
        dest[a].tex_coord.x = ((1.0*src[a].x) / (1.0 * (map.x_size * sw_scale)));
        dest[a].tex_coord.y = ((1.0*src[a].y) / (1.0 * (map.y_size * sw_scale)));
        //while (dest[a].tex_coord.x > 1) dest[a].tex_coord.x -= 1.0;
        //while (dest[a].tex_coord.y > 1) dest[a].tex_coord.y -= 1.0;
        //while (dest[a].tex_coord.x < 0) dest[a].tex_coord.x += 1.0;
        //while (dest[a].tex_coord.y < 0) dest[a].tex_coord.y += 1.0;
        printf("tex_coord = %0.3f, %0.3f\n", dest[a].tex_coord.x, dest[a].tex_coord.y);
    }
    return (dest);
} 

SDL_Vertex *clip_vertexes(SDL_Vertex *src, int *num) {
    SDL_Vertex *dest;
    float slope = 0.0;
    int a;
    int dest_num = 0;
    int clips;
    int top_y, right_x, bottom_y, left_x;
    if (num==0 || src==NULL) return NULL;
    dest = malloc(20 * sizeof(SDL_Vertex));
    if (dest==NULL) {printf("malloc failure\n");exit(-1);}

    // WORK IN PROGRESS //
    top_y = 0 + map_origin_y;
    right_x = map.x_size * sw_scale + map_origin_x;
    bottom_y = map.y_size * sw_scale + map_origin_y;
    left_x = 0 + map_origin_x;

    for (a=0; a<*num; a++) {
        clips = 0;
        
        if (src[a].position.y < top_y) {
            // clip top;
            if (clips==0) {
                dest[dest_num] = src[a]; // copy then modify
                dest[dest_num+1] = src[a]; // copy then modify
                slope = (src[a].position.y - src[a-1].position.y) / (src[a].position.x - src[a-1].position.x);
                printf("src[a] = %f, %f    src[a-1] = %f, %f\n",src[a].position.x,src[a].position.y,
                                                                src[a-1].position.x,src[a-1].position.y);
            } else  {
                dest_num-=2;
            };
            dest[dest_num].position.x= src[a].position.x - ((src[a].position.y-top_y) / slope);
            dest[dest_num].position.y=top_y;
            //dest[dest_num+1].position.x=0;    // UNCHANGED
            dest[dest_num+1].position.y=top_y;
            dest[dest_num].tex_coord.x = ((1.0*dest[dest_num].position.x / (1.0 * ((map.x_size) * sw_scale))));
            dest[dest_num].tex_coord.y = ((1.0*dest[dest_num].position.y / (1.0 * ((map.y_size) * sw_scale))));
            dest[dest_num+1].tex_coord.x = ((1.0*dest[dest_num+1].position.x / (1.0 * ((map.x_size) * sw_scale))));
            dest[dest_num+1].tex_coord.y = ((1.0*dest[dest_num+1].position.y / (1.0 * ((map.y_size) * sw_scale))));
            printf("[%d] Clip top    for point %f,%f (slope = %f)\n",a,src[a].position.x,src[a].position.y, slope);
            printf("                 pos[%d] = %f,%f\n",dest_num,dest[dest_num].position.x,dest[dest_num].position.y);
            printf("                 pos[%d] = %f,%f\n",dest_num+1,dest[dest_num+1].position.x,dest[dest_num+1].position.y);
            printf("tex_coord = %0.3f, %0.3f\n", dest[dest_num].tex_coord.x, dest[dest_num].tex_coord.y);
            printf("tex_coord = %0.3f, %0.3f\n", dest[dest_num+1].tex_coord.x, dest[dest_num+1].tex_coord.y);
            dest_num += 2;
            clips ++;
        }

        if (src[a].position.x >= right_x) {
            // clip right
            if (clips==0) {
                dest[dest_num] = src[a]; // copy then modify
                dest[dest_num+1] = src[a]; // copy then modify
                slope = (src[a].position.y - src[a-1].position.y) / (src[a].position.x - src[a-1].position.x);
                printf("src[a] = %f, %f    src[a-1] = %f, %f\n",src[a].position.x,src[a].position.y,
                                                                src[a-1].position.x,src[a-1].position.y);
            } else  {
                dest_num-=2;
            };
            dest[dest_num].position.x=right_x;
            dest[dest_num].position.y=src[a].position.y + (((map.x_size*sw_scale) - src[a].position.x) / slope);
            dest[dest_num+1].position.x=right_x;
            //dest[dest_num+1].position.y=(map.y_size)*sw_scale;    // unchanged
            dest[dest_num].tex_coord.x = ((1.0*dest[dest_num].position.x / (1.0 * ((map.x_size) * sw_scale))));
            dest[dest_num].tex_coord.y = ((1.0*dest[dest_num].position.y / (1.0 * ((map.y_size) * sw_scale))));
            dest[dest_num+1].tex_coord.x = ((1.0*dest[dest_num+1].position.x / (1.0 * ((map.x_size) * sw_scale))));
            dest[dest_num+1].tex_coord.y = ((1.0*dest[dest_num+1].position.y / (1.0 * ((map.y_size) * sw_scale))));
            printf("[%d] Clip right  for point %f,%f (slope = %f)\n",a,src[a].position.x,src[a].position.y, slope);
            printf("                 pos[%d] = %f,%f\n",dest_num,dest[dest_num].position.x,dest[dest_num].position.y);
            printf("                 pos[%d] = %f,%f\n",dest_num+1,dest[dest_num+1].position.x,dest[dest_num+1].position.y);
            printf("tex_coord = %0.3f, %0.3f\n", dest[dest_num].tex_coord.x, dest[dest_num].tex_coord.y);
            printf("tex_coord = %0.3f, %0.3f\n", dest[dest_num+1].tex_coord.x, dest[dest_num+1].tex_coord.y);
            dest_num += 2;
            clips ++;
        }
        if (src[a].position.y >= bottom_y ) {
            // clip bottom
            if (clips==0) {
                dest[dest_num] = src[a]; // copy then modify
                dest[dest_num+1] = src[a]; // copy then modify
                slope = (src[a].position.y - src[a-1].position.y) / (src[a].position.x - src[a-1].position.x);
                printf("src[a] = %f, %f    src[a-1] = %f, %f\n",src[a].position.x,src[a].position.y,
                                                                src[a-1].position.x,src[a-1].position.y);
            } else  {
                dest_num-=2;
            };
            dest[dest_num].position.x=src[a].position.x + ((src[a].position.y-bottom_y) * slope);
            dest[dest_num].position.y=bottom_y;
            //dest[dest_num+1].position.x=(map.x_size)*sw_scale; // unchanged
            dest[dest_num+1].position.y=bottom_y;
            dest[dest_num].tex_coord.x = ((1.0*dest[dest_num].position.x / (1.0 * ((map.x_size) * sw_scale))));
            dest[dest_num].tex_coord.y = ((1.0*dest[dest_num].position.y / (1.0 * ((map.y_size) * sw_scale))));
            dest[dest_num+1].tex_coord.x = ((1.0*dest[dest_num+1].position.x / (1.0 * ((map.x_size) * sw_scale))));
            dest[dest_num+1].tex_coord.y = ((1.0*dest[dest_num+1].position.y / (1.0 * ((map.y_size) * sw_scale))));
            printf("[%d] Clip bottom for point %f,%f (slope = %f)\n",a,src[a].position.x,src[a].position.y, slope);
            printf("                 pos[%d] = %f,%f\n",dest_num,dest[dest_num].position.x,dest[dest_num].position.y);
            printf("                 pos[%d] = %f,%f\n",dest_num+1,dest[dest_num+1].position.x,dest[dest_num+1].position.y);
            printf("tex_coord = %0.3f, %0.3f\n", dest[dest_num].tex_coord.x, dest[dest_num].tex_coord.y);
            printf("tex_coord = %0.3f, %0.3f\n", dest[dest_num+1].tex_coord.x, dest[dest_num+1].tex_coord.y);
            dest_num += 2;
            clips ++;
        }
        if (src[a].position.x < left_x) {
            // clip left;
            if (clips==0) {
                dest[dest_num] = src[a]; // copy then modify
                dest[dest_num+1] = src[a]; // copy then modify
                slope = (src[a].position.y - src[a-1].position.y) / (src[a].position.x - src[a-1].position.x);
                printf("src[a] = %f, %f    src[a-1] = %f, %f\n",src[a].position.x,src[a].position.y,
                                                                src[a-1].position.x,src[a-1].position.y);
            } else  {
                dest_num-=2;
            };
            dest[dest_num].position.x=left_x;
            dest[dest_num].position.y= + src[a].position.y + (src[a].position.x / slope);
            dest[dest_num+1].position.x=left_x;
            //dest[dest_num+1].position.y=0; // unchanged
            dest[dest_num].tex_coord.x = ((1.0*dest[dest_num].position.x / (1.0 * ((map.x_size) * sw_scale))));
            dest[dest_num].tex_coord.y = ((1.0*dest[dest_num].position.y / (1.0 * ((map.y_size) * sw_scale))));
            dest[dest_num+1].tex_coord.x = ((1.0*dest[dest_num+1].position.x / (1.0 * ((map.x_size) * sw_scale))));
            dest[dest_num+1].tex_coord.y = ((1.0*dest[dest_num+1].position.y / (1.0 * ((map.y_size) * sw_scale))));
            printf("[%d] Clip left   for point %f,%f (slope = %f)\n",a,src[a].position.x,src[a].position.y, slope);
            printf("                 pos[%d] = %f,%f\n",dest_num,dest[dest_num].position.x,dest[dest_num].position.y);
            printf("                 pos[%d] = %f,%f\n",dest_num+1,dest[dest_num+1].position.x,dest[dest_num+1].position.y);
            printf("tex_coord = %0.3f, %0.3f\n", dest[dest_num].tex_coord.x, dest[dest_num].tex_coord.y);
            printf("tex_coord = %0.3f, %0.3f\n", dest[dest_num+1].tex_coord.x, dest[dest_num+1].tex_coord.y);
            dest_num += 2;
            clips ++;
        }
    
        if (clips==0) {
            // no clipping needed
            dest[dest_num] = src[a];
            printf("[%d] No clipping needed for point %f,%f\n",a,src[a].position.x,src[a].position.y);
            printf("                 pos[%d] = %f,%f\n",dest_num,dest[dest_num].position.x,dest[dest_num].position.y);
            printf("tex_coord = %0.3f, %0.3f\n", dest[dest_num].tex_coord.x, dest[dest_num].tex_coord.y);
            dest_num += 1;
        }
        if (dest[dest_num-1].tex_coord.x <0 || dest[dest_num-1].tex_coord.x > 1 ) {
                printf("tex_coord out of range\n"); //exit(-1);
        }
        if (dest[dest_num-1].tex_coord.y <0 || dest[dest_num-1].tex_coord.y > 1 ) {
                printf("tex_coord out of range\n"); //exit(-1);
        }
    }
    printf("num changed from %d to",*num);
    *num = dest_num;    // return num_vertices of dest
    printf(" %d\n",*num);
    return(dest);
}


// takes an F_Point list with dir, location, and size and draws it.
void sdl_draw_transformed_polygon(struct F_Point_s *symbol_points, int pcount, int dir, float tile_x, float tile_y, float size_x, float size_y){
    // Draw Player
    struct F_Point_s *ptemp0, *ptemp1, *ptemp2, *ptemp3;
    SDL_Point *scr_points;
    SDL_Vertex *scr_vertices, *scr_vertices2;
    const int indices[] = { 0,1,2,  0,2,3,  0,3,4,  0,4,5,  0,5,6,  0,6,7,  0,7,8,  0,8,9 }; // Can keep increasing this for larger polygons
    ptemp0 = scale_to_center_points(symbol_points,pcount, size_x,size_y);
    ptemp1 = rotate_points(ptemp0,pcount, dir);
    ptemp2 = scale_points(ptemp1,pcount, sw_scale, sw_scale);
    ptemp3 = move_points(ptemp2,pcount, tile_x, tile_y);
    scr_points = convert_points(ptemp3,pcount);
        //SDL_RenderDrawLines(renderer,scr_points,pcount); 
    scr_vertices = convert_scr_points_to_vertexes(scr_points,pcount, COLOR_GREY_D);
    scr_vertices2 = clip_vertexes(scr_vertices,&pcount);    // Note: pcount gets updated
    if (scr_vertices2 != NULL){
        for (int z=0; z<(pcount); z++) {
            printf("Render Geometry vertex %d %f,%f:\n",z, scr_vertices2[z].position.x, scr_vertices2[z].position.y);
        }
        if (pcount >= 3 && pcount <=10) { // draw a series of triangles to complete the polygon.
            SDL_RenderGeometry(renderer, texture, scr_vertices2, pcount, indices, (pcount-2)*3);
            printf("sdl_draw_transformed_polygon(%d->%d indices)\n",pcount, (pcount-2)*3);
        } else {
            printf("sdl_draw_transformed_polygon() only supports 3-10 sided shapes. %d were requested\n",pcount);
        };
        free(ptemp0); free(ptemp1); free(ptemp2); free(ptemp3);
        free(scr_points); free(scr_vertices); free(scr_vertices2);
    } else {
        printf("scr_vertices2 == NULL\n");
        exit(-1);
    }
}

void test_clipper(void) {
   SDL_Vertex polygon[4];
   SDL_Rect   clip_rect;
   int vcount = 4;
   const int indices[] = { 0,1,2,  0,2,3,  0,3,4,  0,4,5,  0,5,6,  0,6,7,  0,7,8,  0,8,9 }; // Can keep increasing this for larger polygons

   polygon[0].position.x = 100;    polygon[0].position.y = 100;
   polygon[1].position.x = 200;    polygon[1].position.y = 100;
   polygon[2].position.x = 200;    polygon[2].position.y = 200;
   polygon[3].position.x = 100;    polygon[3].position.y = 200;
   clip_rect.x = 50;  clip_rect.y = 50;
   clip_rect.h = 200; clip_rect.h = 200;
   SDL_RenderGeometry(renderer, texture, polygon, vcount, indices, (vcount-2)*3);

}
// -------------------------------------

// convert screen coordinates to map coordinates
struct F_Point_s convert_scr_to_map(int scr_x, int scr_y) {
    struct F_Point_s map_loc;
    map_loc.x = ( scr_x - map_origin_x ) / sw_scale;
    map_loc.y = ( scr_y - map_origin_y ) / sw_scale;
    printf("scr %d,%d -> map %f,%f\n",scr_x, scr_y, map_loc.x, map_loc.y);
    return map_loc;
}

// simply makes an F_Rect structure
struct F_Rect_s make_F_Rect(float x, float y, float size_x, float size_y) {
    struct F_Rect_s frect;
    frect.x = x;
    frect.y = y;
    frect.w = size_x;
    frect.h = size_y;
    return frect;
}

// simply makes an F_Point structure
struct F_Point_s make_F_Point(float x, float y) {
    struct F_Point_s fpoint;
    fpoint.x = x;
    fpoint.y = y;
    return fpoint;
}

// print F_Rect list for debug
void print_frects(struct F_Rect_s *rlist, int rcnt) {
    for (int i=0;i<rcnt;i++) {
        printf("i=%d: x=%f, y=%f, w=%f, h=%f\n", i, rlist[i].x, rlist[i].y, rlist[i].w, rlist[i].h);
    }
}

// print SDL_Rect list for debug
void print_srects(SDL_Rect *rlist, int rcnt) {
    for (int i=0;i<rcnt;i++) {
        printf("i=%d: x=%d, y=%d, w=%d, h=%d\n", i, rlist[i].x, rlist[i].y, rlist[i].w, rlist[i].h);
    }
}

int sdl_draw_shadows(float x_center, float y_center) {
    // variables for drawing walls
    int           x,y;        // map location we are working on
    int           dir;        // direction of wall we are working on
    unsigned char walls;      // temporary hold walls[x][y]
    unsigned char both_bits;  // temporary doors and walls of current tile
    unsigned char visible;    // map visibility of current tile

    // variables for drawing shadows
    struct F_Point_s line[4];

    // variables used in both drawing walls and symbols
    float scr_center_x, scr_center_y; // in screen coordinates
    float map_center_x, map_center_y; // in map coordinates

    float wall_end_a_x, wall_end_a_y;
    float wall_end_b_x, wall_end_b_y;
    float slope_a, slope_b;

    // calculate map origin in screen coordinates
    scr_center_x = ((float)screen_width) / 2.0;
    scr_center_y = ((float)screen_height) / 2.0;
    map_center_x = x_center;
    map_center_y = y_center;
    map_origin_x = scr_center_x - (map_center_x * sw_scale);
    map_origin_y = scr_center_y - (map_center_y * sw_scale);

    for (y=0; y<map.y_size; y++) {  // x,y in integer map coordinates
        for (x=0; x<map.x_size; x++) {
            walls = map.walls[x][y];
            visible = map.visible[x][y];

            if (visible>0 && visible<255) {
                // draw shadows behind all visible walls.
                SDL_SetRenderDrawColor(renderer, COLOR_WHITE);
                for (dir=0;dir<=3;dir++) {  // TODO
                    both_bits = get_both_bits(walls,dir);
                    //printf("both_bits: walls=%x, dir=%d\n",walls,dir);
                    switch(both_bits) {
                    case 0x00: // open
                    case 0x10: // open_door
                        // do nothing
                        break;
                    case 0x01: // wall
                    case 0x11: // closed_door
                        if ((dir==0) && (pl.y>y) && (y>0)) {
                            // world map coordinate calculations
                            wall_end_a_x = x;
                            wall_end_a_y = y;
                            wall_end_b_x = x+1;
                            wall_end_b_y = y;
                            slope_a = (wall_end_a_y - pl.y) / (wall_end_a_x - pl.x);
                            slope_b = (wall_end_b_y - pl.y) / (wall_end_b_x - pl.x);
                            // cell relative coordinates:
                            line[0].x = 0;  // end of wall segment
                            line[0].y = 0;  // end of wall segment
                            line[1].x = -1/slope_a * y; // edge of map
                            line[1].y = -1 * y;         // edge of map

                            line[3].x = 1;
                            line[3].y = 0;
                            line[2].x = -1/slope_b * y + 1;
                            line[2].y = -1 * y;

                            // TODO: make this a filled polygon with all 4 vertices
                            //SDL_SetRenderDrawColor(renderer, COLOR_RED);
                            sdl_draw_transformed_polygon(line,  4,0,map_origin_x+(x*sw_scale),map_origin_y+(y*sw_scale),1,1);
                            sdl_draw_transformed_lines(line,    4,0,map_origin_x+(x*sw_scale),map_origin_y+(y*sw_scale),1,1);
                            break;
                        }
                        if ((dir==1) && (pl.x<x+1) && (x<map.x_size-1)) {
                            // world map coordinate calculations
                            wall_end_a_x = x+1;
                            wall_end_a_y = y;
                            wall_end_b_x = x+1;
                            wall_end_b_y = y+1;
                            slope_a = (wall_end_a_y - pl.y) / (wall_end_a_x - pl.x);
                            slope_b = (wall_end_b_y - pl.y) / (wall_end_b_x - pl.x);
                            // cell relative map coordinates
                            line[0].x = 1;
                            line[0].y = 0;
                            line[1].x = (map.x_size - x);
                            line[1].y = (slope_a) * (map.x_size - x - 1);

                            line[3].x = 1;
                            line[3].y = 1;
                            line[2].x = (map.x_size - x );
                            line[2].y = (slope_b) * (map.x_size - x -1) +1;

                            // TODO: make this a filled polygon with all 4 vertices
                            //SDL_SetRenderDrawColor(renderer, COLOR_BLUE);
                            sdl_draw_transformed_polygon(line,  4,0,map_origin_x+(x*sw_scale),map_origin_y+(y*sw_scale),1,1);
                            sdl_draw_transformed_lines(line,    4,0,map_origin_x+(x*sw_scale),map_origin_y+(y*sw_scale),1,1);
                            break;
                        }
                        if ((dir==2) && (pl.y<y+1) && (y<map.y_size-1)) {
                            // map coordinate calculations
                            wall_end_a_x = x+1;
                            wall_end_a_y = y+1;
                            wall_end_b_x = x;
                            wall_end_b_y = y+1;
                            slope_a = (wall_end_a_y - pl.y) / (wall_end_a_x - pl.x);
                            slope_b = (wall_end_b_y - pl.y) / (wall_end_b_x - pl.x);
                            //cell relative map coordinates
                            line[0].x = 1;
                            line[0].y = 1;
                            line[1].x = 1/slope_a * (map.y_size - y-1)+1 ;
                            line[1].y = (map.y_size - y);

                            line[3].x = 0;
                            line[3].y = 1;
                            line[2].x = 1/slope_b * (map.y_size - y-1) ;
                            line[2].y = (map.y_size - y);
                            // TODO: make this a filled polygon with all 4 vertices
                            //SDL_SetRenderDrawColor(renderer, COLOR_GREEN);
                            sdl_draw_transformed_polygon(line,  4,0,map_origin_x+(x*sw_scale),map_origin_y+(y*sw_scale),1,1);
                            sdl_draw_transformed_lines(line,    4,0,map_origin_x+(x*sw_scale),map_origin_y+(y*sw_scale),1,1);
                            break;
                        }
                        if ((dir==3) && (pl.x>x) && (x>0)) {
                            // map coordinate calculations
                            wall_end_a_x = x;
                            wall_end_a_y = y;
                            wall_end_b_x = x;
                            wall_end_b_y = y+1;
                            slope_a = (wall_end_a_y - pl.y) / (wall_end_a_x - pl.x);
                            slope_b = (wall_end_b_y - pl.y) / (wall_end_b_x - pl.x);
                            //cell relative map coordinates
                            line[0].x = 0;
                            line[0].y = 0;
                            line[1].x = -x;
                            line[1].y = -slope_a * x;

                            line[3].x = 0;
                            line[3].y = 1;
                            line[2].x = -1 * x;
                            line[2].y = -slope_b * x + 1;

                            // TODO: make this a filled polygon with all 4 vertices
                            //SDL_SetRenderDrawColor(renderer, COLOR_WHITE);
                            sdl_draw_transformed_polygon(line,  4,0,map_origin_x+(x*sw_scale),map_origin_y+(y*sw_scale),1,1);
                            sdl_draw_transformed_lines(line,    4,0,map_origin_x+(x*sw_scale),map_origin_y+(y*sw_scale),1,1);
                            break;
                        }
                    }
                }
            }
        }
    }
    return(0);
}

// small helper alias that takes a color struct instead of the individual parts.
int SetRenderDrawColorPct(SDL_Renderer *renderer, SDL_Color c, float pct) {
    return SDL_SetRenderDrawColor(renderer, iround(pct*c.r), iround(pct*c.g), iround(pct*c.b), iround(pct*c.a));
}

// main map view drawing routine
// TODO style: for texture map use 0x01111; for viewport = 0x11101
//   bit[4] = include player
//   bit[3] = include objects
//   bit[2] = include known map areas (always medium grey)
//   bit[1] = color of visible areas (1=medium grey,2=light grey)
//   bit[0] = include visible areas

int sdl_draw_view(float x_center, float y_center, int style) {
    sdl_draw_map(x_center, y_center, style);
    sdl_draw_objects(x_center, y_center, style);
    sdl_draw_monsters(x_center, y_center, style);
    sdl_draw_player(x_center, y_center, style);
    if (status==battle) draw_battlemap();
    return (1);
}

// TODO: implement style?
int sdl_draw_map(float x_center, float y_center, int style) {
    // variables for drawing walls
    int           x,y;        // map location we are working on
    int           dir;        // direction of wall we are working on
    float         scr_x,scr_y;// screen location (pixel coordinates)
    unsigned char walls;      // temporary hold walls[x][y]
    unsigned char both_bits;  // temporary doors and walls of current tile
    unsigned char visible;    // map visibility of current tile
    unsigned char tnum;
    bool          off_map;    // location is off the map or invalid

    struct F_Rect_s *rtemp1;
    int           rcount;      // number of rectangles in current symbol (rtemp*, scr_rects)

    // variables used in both drawing walls and symbols
    float scr_center_x, scr_center_y; // in screen coordinates
    float map_center_x, map_center_y; // in map coordinates

    // calculate map origin in screen coordinates
    scr_center_x = ((float)screen_width) / 2.0;
    scr_center_y = ((float)screen_height) / 2.0;
    map_center_x = x_center;
    map_center_y = y_center;
    map_origin_x = scr_center_x - (map_center_x * sw_scale);
    map_origin_y = scr_center_y - (map_center_y * sw_scale);

    // set background color for entire window
    dirty_display = 0;
    SDL_SetRenderDrawColor(renderer, COLOR_GREY_D);
    SDL_RenderClear(renderer);

    // First pass, just draw floors.
    for (y=0; y<map.y_size; y++) {  // x,y in integer map coordinates
        for (x=0; x<map.x_size; x++) {
            walls = map.walls[x][y];
            scr_x = map_origin_x + (x * sw_scale);
            scr_y = map_origin_y + (y * sw_scale);
            off_map = 1-map.valid[x][y];
            visible = map.visible[x][y];
            tnum = map.walls[x][y]>>12 & 0xF;
            //if (terrain !=0) printf("Terrain floor = %d, x,y=%d,%d\n",terrain,x,y);
            // clear the tile (draw the floor)
            if (off_map || visible==0) {  //TODO: Add style
                SDL_SetRenderDrawColor(renderer, COLOR_GREY_D);
            } else if (visible==255) {
                SetRenderDrawColorPct(renderer, map.terrains[tnum].c_floor,0.6);
                //SDL_SetRenderDrawColor(renderer, COLOR_GREY_M);
            } else {
                SetRenderDrawColorPct(renderer, map.terrains[tnum].c_floor,1.0);
                //printf("FloorT[%d]=%02x,%02x,%02x,%02x\n",tnum,map.terrains[tnum].c_floor.r,map.terrains[tnum].c_floor.g,map.terrains[tnum].c_floor.b,map.terrains[tnum].c_floor.a);
            }
            sdl_draw_transformed_rects(cell_floor,cell_floor_rcount,0,scr_x,scr_y,1.01,1.01);
        }
    }
    // Second Pass, just draw special walls
    //if (map.terrains[0].wall_texture != NULL) {
        for (y=0; y<map.y_size; y++) {  // x,y in integer map coordinates
            for (x=0; x<map.x_size; x++) {
                walls = map.walls[x][y];
                scr_x = map_origin_x + ((x+0.5) * sw_scale);
                scr_y = map_origin_y + ((y+0.5) * sw_scale);
                off_map = 1-map.valid[x][y];
                visible = map.visible[x][y];
                tnum = map.walls[x][y]>>12 & 0xF;
                //if (terrain !=0) printf("Terrain walls = %d, x,y=%d,%d\n",terrain,x,y);
                if (!(off_map || visible==0)) {
                    // draw the wall symbols: perimiter 4 sides (walls or doors)
                    for (dir=0;dir<=3;dir++) {
                        both_bits = get_both_bits_xy(x,y,dir);
                        //printf("both_bits: walls=%x, dir=%d\n",walls,dir);
                        if (both_bits == 0x01) {
                            draw_image(map.terrains[tnum].wall_texture,
                                    scr_x, scr_y,
                                    sw_scale, sw_scale,
                                    dir, x+y);
                        }
                        if (both_bits == 0x10) {
                            draw_image(map.terrains[tnum].door_open_texture,
                                    scr_x, scr_y,
                                    sw_scale, sw_scale,
                                    dir, x+y);
                        }
                        if (both_bits == 0x11) {
                            draw_image(map.terrains[tnum].door_closed_texture,
                                    scr_x, scr_y,
                                    sw_scale, sw_scale,
                                    dir, x+y);
                        }
                    }
                }
            }
        }
    //}
    // Third Pass, just draw walls/doors lines
    for (y=0; y<map.y_size; y++) {  // x,y in integer map coordinates
        for (x=0; x<map.x_size; x++) {
            walls = map.walls[x][y];
            scr_x = map_origin_x + (x * sw_scale);
            scr_y = map_origin_y + (y * sw_scale);
            off_map = 1-map.valid[x][y];
            visible = map.visible[x][y];
            tnum = map.walls[x][y]>>12 & 0xF;
            //if (terrain !=42) printf("Terrain wall lines = %d, x,y=%d,%d\n",terrain,x,y);
            if (!(off_map || visible==0)) {
                // draw the perimiter 4 sides (walls or doors)
                if (visible==2) {
                    SetRenderDrawColorPct(renderer, map.terrains[tnum].c_wall,0.6);
                } else {
                    SetRenderDrawColorPct(renderer, map.terrains[tnum].c_wall,1.0);
                }
                for (dir=0;dir<=3;dir++) {
                    both_bits = get_both_bits_xy(x,y,dir);
                    //printf("both_bits: walls=%x, dir=%d\n",walls,dir);
                    switch(both_bits) {
                    case 0x00: // open
                        rtemp1 = NULL;
                        rcount = 0;
                        //printf("Wall open @ dir=%d x=%d, y=%d\n",dir,x,y);
                        break;
                    case 0x01: // wall
                        rtemp1 = wall_solid;
                        rcount = wall_solid_rcount;
                        //printf("Wall solid @ dir=%d\n",dir);
                        break; 
                    case 0x10: // open_door
                        rtemp1 = door_open;
                        rcount = door_open_rcount;
                        //printf("Door open @ dir=%d\n",dir);
                        break;
                    case 0x11: // closed_door
                        rtemp1 = door_closed;
                        rcount = door_closed_rcount;
                        //printf("Door closed @ dir=%d\n",dir);
                        break;
                    default: exit(-1);
                    }
                    sdl_draw_transformed_rects(rtemp1,rcount, dir, scr_x, scr_y, 1.01,1.01);
                }
            }
        }
    }
    return (1);
}

int sdl_draw_objects(float x_center, float y_center, int style) {
    // variables for drawing symbols
    struct F_Rect_s *rtemp1;
    int           rcount;      // number of rectangles in current symbol (rtemp*, scr_rects)
    float         size;        // size of symbol (usually < 1.0)
    int           dir;        // direction of wall we are working on
    //float         scr_x,scr_y;// screen location (pixel coordinates)

    // Draw Objects ; TODO: Use style
    //printf("Draw objects (num_objedts=%d)\n",num_objects);
    for (int z=0; z<num_objects; z++) {
        if (objects[z].valid==1 //&& objects[z].held==0
                && objects[z].owner=='M'
                && objects[z].owner_num == map.map_number
                && map.visible[ifloor(objects[z].x)][ifloor(objects[z].y)]>0){
            //printf("Draw objects %d (%s)\n",z, objects[z].graphic_name);
            if (objects[z].graphic_type==1) {
                draw_image(objects[z].texture,
                        map_origin_x +objects[z].x * sw_scale,
                        map_origin_y +objects[z].y * sw_scale,
                        sw_scale * objects[z].size, sw_scale * objects[z].size,
                        objects[z].dir,0);
            }
            if (objects[z].graphic_type==2) {
                rtemp1 = objects[z].texture;
                rcount = objects[z].frect_count;
                dir =    objects[z].dir;
                size =   objects[z].size;
                sdl_draw_transformed_rects(rtemp1,rcount, dir,
                        map_origin_x +(objects[z].x-0.5) * sw_scale,
                        map_origin_y +(objects[z].y-0.5) * sw_scale,
                        //scr_x, scr_y,
                        size,size);
            }
            // todo.. graphic_type==3 for point based objects, like player arrow
        }
    }
    return(0);
}

int sdl_draw_monsters(float x_center, float y_center, int style) {
    // variables for drawing symbols
    struct F_Rect_s *rtemp1;
    int           rcount;      // number of rectangles in current symbol (rtemp*, scr_rects)
    float         size;        // size of symbol (usually < 1.0)
    int           dir;        // direction of wall we are working on
    //float         scr_x,scr_y;// screen location (pixel coordinates)

    // Draw Objects ; TODO: Use style
    for (int z=0; z<num_monsters; z++) {
        if (monsters[z].valid==1
                && monsters[z].map_number == map.map_number
                && map.visible[ifloor(monsters[z].x)][ifloor(monsters[z].y)]>0
                && map.visible[ifloor(monsters[z].x)][ifloor(monsters[z].y)]<255) {

            // determine valid directions for monster based on option_graphic_dir
            if (monsters[z].graphic_type==1) {
                dir = monsters[z].dir;
                if (monsters[z].option_graphic_dir==2) {
                    if (dir == 3) {
                        dir = -3;  //flip_horizontally
                    } else {
                        dir = 0;
                    }
                } else if (monsters[z].option_graphic_dir==0) {
                    dir=0;
                } else {    //
                    dir =   monsters[z].dir;
                }
                printf("option = %d, dir=%d\n,",monsters[z].option_graphic_dir, dir);
                draw_image(monsters[z].texture,
                        map_origin_x +monsters[z].x * sw_scale,
                        map_origin_y +monsters[z].y * sw_scale,
                        sw_scale * monsters[z].size, sw_scale * monsters[z].size,
                        dir,0);
            }
            if (monsters[z].graphic_type==2) {
                rtemp1 = monsters[z].texture;
                rcount = monsters[z].frect_count;
                dir =    monsters[z].dir;
                size =   monsters[z].size;
                sdl_draw_transformed_rects(rtemp1,rcount, dir,
                        map_origin_x +(monsters[z].x-0.5) * sw_scale,
                        map_origin_y +(monsters[z].y-0.5) * sw_scale,
                        //scr_x, scr_y,
                        size,size);
            }
            // todo.. graphic_type==3 for point based monsters, like player arrow
        }
    }
    return(0);
}

int sdl_draw_player(float x_center, float y_center, int style) {
    float         scr_x,scr_y;// screen location (pixel coordinates)
    // Draw Player ; TODO: Use style
    scr_x = map_origin_x + ((pl.x-0.5) * sw_scale); //TODO: move back to center
    scr_y = map_origin_y + ((pl.y-0.5) * sw_scale);
    sdl_draw_transformed_lines(player_symbol_points, player_symbol_pcount, pl.dir, scr_x, scr_y, 0.5, 0.5);
    return(1);
}

// initialize texture, used for shadow map.  Recreate whenever size_x, size_y changes.
SDL_Texture *create_render_texture (int size_x, int size_y) {
    SDL_Texture *return_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888,
		SDL_TEXTUREACCESS_TARGET, size_x+1, size_y+1);
    if (return_texture == NULL) {
        printf("SDL_CreateTexture() returned NULL: %s\n",SDL_GetError());
        exit(-1);
    }
    SDL_SetTextureBlendMode(return_texture,SDL_BLENDMODE_BLEND);
    return return_texture;
}

// call with NULL to set render target back to window.
int set_render_to_texture (SDL_Texture *texture_target, int clear,
                uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha) {
    SDL_SetRenderTarget(renderer, texture_target); // redirect draw commands to texture
    if (clear == 1) {
        // set background color for entire window
        //SDL_SetRenderDrawColor(renderer, COLOR_GREY_D);
        SDL_SetRenderDrawColor(renderer, red, green, blue, alpha);
        SDL_RenderClear(renderer);
    };
    return 1;
}

//////////////////////////////////////////////////////////
// Main loop functions                                  //
//////////////////////////////////////////////////////////

// Who dirtied the display could be determined by the value of dirty_display.
// Init routine => 3. items => 5, user_input => 1, timer => 2, open chest => 4
// zoom in/out => 6
void dirty_display_handler(void) {
    // check to see if we need to redraw the screen
    // update display if needed, from commands or from timer for animation
    if(dirty_display>0) {
        //printf("Updating display. dirty_display=%d.\n", dirty_display);
        if (texture==NULL) {
            texture = create_render_texture ((map.x_size)*sw_scale, (map.y_size)*sw_scale);
            if (texture == NULL) {
                printf("create_render_texture () returned NULL: %s\n",SDL_GetError());
                exit(-1);
            }
        }
        display_buttons_for_objects();
        //display_messages();

        //player_view_simple();  // determine what squares are 'visible'
        player_view_noback();  // determine what squares are 'visible'
        sdl_draw_view(pl.x,pl.y, 0x1d); // center on player

        if(option_enable_shadows) {
            set_render_to_texture(texture,1,COLOR_BLUE); // begin rendering to texture
            sdl_draw_view(pl.x,pl.y, 0x0f); // center view on player
            //sdl_draw_view(map.x_size/2,map.y_size/2, 0x0f); // center on map
            set_render_to_texture(NULL,1,COLOR_GREY_D);    // end rendering to texture
            sdl_draw_shadows(pl.x,pl.y);
        };

        arrange_and_draw_buttons();
        arrange_and_draw_messages();

        SDL_RenderPresent(renderer);
        num_times_rendered++;
        dirty_display = 0;
    }
}

// start the SDL graphics subsystem
int sdl_start(void) {
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER);
    window = SDL_CreateWindow("Dungeon-Reincarnate-1", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, screen_width, screen_height, SDL_WINDOW_SHOWN);
    if (window==NULL) {printf("SDL_CreateWindow() returned NULL: %s\n",SDL_GetError());}
    SDL_SetWindowResizable(window,1);
    renderer = SDL_CreateRenderer(window, -1, /* 0 */ SDL_RENDERER_PRESENTVSYNC /*SDL_RENDERER_ACCELERATED*/);
    SDL_RenderSetScale(renderer, HW_SCALE, HW_SCALE);
    SDL_SetRenderDrawBlendMode(renderer,SDL_BLENDMODE_BLEND);
    texture = NULL;     // create this when we need it, destroy it when screen size changes.
    dirty_display = 3;  // redraw screen if not 0.
    timer_id = setup_callback_interval(animation_time);
    start_ttf();
    init_messages();
    return (0);
}

// Clean up and close the SDL graphics subsystem
int sdl_quit(void) {
    remove_callback_interval();
    quit_ttf();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    if (texture != NULL) SDL_DestroyTexture(texture);
    SDL_Quit();
    return (0);
}
