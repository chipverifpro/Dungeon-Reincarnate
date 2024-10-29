// mapio.c
#include "all_headers.h"

/*
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <stdbool.h>

#include "dungeon.h"
#include "main.h"
#include "buttons.h"
#include "items.h"
#include "map_io.h"
#include "messages.h"
#include "monster.h"
#include "player.h"
#include "sdl_draw.h"
#include "sdl_symbols.h"
#include "sdl_time.h"
#include "user_input.h"
*/

// define the columns of the CSV file
enum csv_fields {
        CSV_OWNER       = 0,
        CSV_OWNER_NUM   = 1,
        CSV_UID         = 2,
        CSV_LOCX        = 3,
        CSV_LOCY        = 4,
        CSV_DIR         = 5,
        CSV_FEATURE     = 6,
        CSV_TYPE        = 7,
        CSV_NAME        = 8,
        CSV_DESCRIPTION = 9,
        CSV_GRAPHIC_NAME=10,
        CSV_MODE        =11,
        CSV_COLOR_RGBA  =12,
        CSV_SIZE        =13,
        CSV_KEEPOUT     =14,
        CSV_QUANTITY    =15,
        CSV_VALID       =16,
        CSV_BUTTON_TEXT =17,
        CSV_CUSTOM_DATA =18,
        CSV_COMMENT     =19
};

// LOCAL GLOBAL Variables
// global variables used by split_csv_line_offsets()
char csv_num_cells; // count of valid CSV cells in below two variables
char *csv_line;     // line read from csv file, modified to replace , with \0
int offsets[200];   // indexes to modified csv_line for each cell

struct object_s *find_uid_owner(int uid) {
    struct object_s *uid_owner = NULL;
    return (uid_owner);
}

int read_map_csv (int load_map_number) {
    FILE *file;
    int x,y;    // current map location
    uint32_t hex;
    char filename[100];
    int headers_done;
    int tnum;

    // clear the map
    for (y=0;y<64;y++) {
        for(x=0;x<64;x++) {
            map.walls[x][y] = 0;
            map.valid[x][y] = 0;
            map.visible[x][y] = 0;
            //map.terrain[x][y] = 0; // part of walls now.
        }
    }
    snprintf(filename,100,"Maps/map%d.csv",load_map_number);
    file = fopen(filename, "r"); // Open the file in read mode
    if (file == NULL) {
        printf("Could not open file %s\n", filename);
        return 1; // Exit if the file cannot be opened
    }


    // LINE 1 = Map,1,Temple of Water,,,,,,,,,
    csv_line = load_line(file);
    chomp(csv_line);   
    split_csv_line_offsets();
    if (strcmp(&csv_line[offsets[0]],"Map")!=0) {
        printf("Invalid map file %s.  Must begin with the word Map instead of %s\n",filename,&csv_line[offsets[0]]);
//        exit(-1);
    }
    map.map_number = atoi(&csv_line[offsets[1]]);
    map.map_name = str_alloc_copy(&csv_line[offsets[2]]);
    current_map_num = map.map_number;
    //map.wall_fname = str_alloc_copy(&csv_line[offsets[3]]);
    switch (map.map_number) {
    //case 1: map.wall_mode = 1; map.c_wall = color_from_rgba (0x00,0x00,0x20,0x20); break;
    //case 2: map.wall_mode = 0; map.c_wall = color_from_rgba (COLOR_BLACK); break;
    //case 3: map.wall_mode = 2; map.c_wall = color_from_rgba (COLOR_BLACK); break;
    }
    //if (strlen(map.walls_fname)>1) { // trailing quote might be left if empty string TODO:FIX
        //map.walls_texture = load_symbol_texture(map.walls_fname, map.c_wall ,map.walls_mode);
    //} else {
        //map.walls_texture = NULL;
    //}

    // clear the terrains, ready to load new ones.
    for (tnum=0;tnum<10;tnum++) {
        map.terrains[tnum].wall_mode = map.terrains[tnum].floor_mode = map.terrains[tnum].door_mode = 0;
        map.terrains[tnum].NameT = NULL;
        map.terrains[tnum].wall_texture = map.terrains[tnum].floor_texture = map.terrains[tnum].door_open_texture = map.terrains[tnum].door_closed_texture = NULL;
    };
    headers_done = 0;
    while (!headers_done) {
        csv_line = load_line(file);
        chomp(csv_line);   
        printf("csv_line = %s\n",csv_line);
        split_csv_line_offsets();
        if (strcmp(&csv_line[offsets[0]],"Size")==0) {
            // Size,size_x,size_y
            map.x_size = atoi(&csv_line[offsets[1]]);
            map.y_size = atoi(&csv_line[offsets[2]]);
            printf(" Size = %d, %d\n", map.x_size, map.y_size);

        } else if (strcmp(&csv_line[offsets[0]],"Start")==0) {
            // starting location specified, only relevant on map 1, otherwise overridden
            map.start_x=atof(&csv_line[offsets[1]]);
            map.start_y=atof(&csv_line[offsets[2]]);
            map.start_dir=atoi(&csv_line[offsets[3]]);
            player_move_to(map.start_x, map.start_y, map.start_dir);
            printf(" Start = %f, %f, dir = %d\n", map.start_x, map.start_y, map.start_dir);

        } else if (strncmp(&csv_line[offsets[0]],"NameT",5)==0) {
            tnum = atoi(&csv_line[offsets[0]+6]);
            printf("NameT[%d]\n", tnum);
            // WallT,mode,filename,r,g,b,a
            map.terrains[tnum].NameT = str_alloc_copy(&csv_line[offsets[1]]);

        } else if (strncmp(&csv_line[offsets[0]],"WallT",5)==0) {
            tnum = atoi(&csv_line[offsets[0]]+6);
            printf("WallT[%d]\n", tnum);
            // WallT,mode,filename,r,g,b,a
            map.terrains[tnum].wall_mode = atoi(&csv_line[offsets[1]]);
            hex = parse_hex(&csv_line[offsets[2]]);
            map.terrains[tnum].wall_fname = str_alloc_copy(&csv_line[offsets[3]]);
            map.terrains[tnum].c_wall.r = hex>>24 & 0xFF;
            map.terrains[tnum].c_wall.g = hex>>16 & 0xFF;
            map.terrains[tnum].c_wall.b = hex>> 8 & 0xFF;
            map.terrains[tnum].c_wall.a = hex     & 0xFF;
            if (strlen(map.terrains[tnum].wall_fname)>1) { // trailing quote might be left if empty string TODO:FIX
                map.terrains[tnum].wall_texture = load_symbol_texture(map.terrains[tnum].wall_fname, map.terrains[tnum].c_wall, map.terrains[tnum].wall_mode);
                printf("loaded wall_texture wall_fname=%s, wall_texture=%p, wall_mode=%d\n",map.terrains[tnum].wall_fname, map.terrains[tnum].wall_texture, map.terrains[tnum].wall_mode);
            } else {
                map.terrains[tnum].wall_texture = NULL;
                map.terrains[tnum].wall_mode = 0;
                printf("wall_fname=%s, wall_texture=%p, wall_mode=%d\n", map.terrains[tnum].wall_fname, map.terrains[tnum].wall_texture, map.terrains[tnum].wall_mode);
            }
            printf(" wall_mode = %d, wall_fname = %s, wall_texture=%p c_wall=%d,%d,%d,%d\n", map.terrains[tnum].wall_mode, map.terrains[tnum].wall_fname, map.terrains[tnum].wall_texture, map.terrains[tnum].c_wall.r, map.terrains[tnum].c_wall.g, map.terrains[tnum].c_wall.b, map.terrains[tnum].c_wall.a);

        } else if (strncmp(&csv_line[offsets[0]],"DoorT",5)==0) {
            tnum = atoi(&csv_line[offsets[0]]+6);
            printf("DoorT[%d]\n", tnum);
            // DoorT,mode,open_filename,closed_filename,r,g,b,a
            map.terrains[tnum].door_mode = atoi(&csv_line[offsets[1]]);
            hex = parse_hex(&csv_line[offsets[2]]);
            map.terrains[tnum].door_open_fname   = str_alloc_copy(&csv_line[offsets[3]]);
            map.terrains[tnum].door_closed_fname = str_alloc_copy(&csv_line[offsets[4]]);
            map.terrains[tnum].c_door.r = hex>>24 & 0xFF;
            map.terrains[tnum].c_door.g = hex>>16 & 0xFF;
            map.terrains[tnum].c_door.b = hex>> 8 & 0xFF;
            map.terrains[tnum].c_door.a = hex     & 0xFF;
            map.terrains[tnum].door_open_texture = NULL;
            map.terrains[tnum].door_closed_texture = NULL;
            if (strlen(map.terrains[tnum].door_open_fname)>1) { // trailing quote might be left if empty string TODO:FIX
                map.terrains[tnum].door_open_texture   = load_symbol_texture(map.terrains[tnum].door_open_fname,   map.terrains[tnum].c_door, map.terrains[tnum].door_mode);
            }
            if (strlen(map.terrains[0].door_closed_fname)>1) { // trailing quote might be left if empty string TODO:FIX
                map.terrains[tnum].door_closed_texture = load_symbol_texture(map.terrains[tnum].door_closed_fname, map.terrains[tnum].c_door, map.terrains[tnum].door_mode);
            }
            printf(" door_mode = %d, door_open_fname   = %s, c_door=%d,%d,%d,%d\n", map.terrains[tnum].door_mode, map.terrains[tnum].door_open_fname, map.terrains[tnum].c_door.r, map.terrains[tnum].c_door.g, map.terrains[tnum].c_door.b, map.terrains[tnum].c_door.a);
            printf(" door_mode = %d, door_closed_fname = %s, c_door=%d,%d,%d,%d\n", map.terrains[tnum].door_mode, map.terrains[tnum].door_closed_fname, map.terrains[tnum].c_door.r, map.terrains[tnum].c_door.g, map.terrains[tnum].c_door.b, map.terrains[tnum].c_door.a);

        } else if (strncmp(&csv_line[offsets[0]],"FloorT",6)==0) {
            tnum = atoi(&csv_line[offsets[0]]+7);
            // FloorT,mode,filename,r,g,b,a
            map.terrains[tnum].floor_mode = atoi(&csv_line[offsets[1]]);
            hex = parse_hex(&csv_line[offsets[2]]);
            map.terrains[tnum].floor_fname = str_alloc_copy(&csv_line[offsets[3]]);
            map.terrains[tnum].c_floor.r = hex>>24 & 0xFF;
            map.terrains[tnum].c_floor.g = hex>>16 & 0xFF;
            map.terrains[tnum].c_floor.b = hex>> 8 & 0xFF;
            map.terrains[tnum].c_floor.a = hex     & 0xFF;
            if (strlen(map.terrains[tnum].floor_fname)>1) { // trailing quote might be left if empty string TODO:FIX
                map.terrains[tnum].floor_texture = load_symbol_texture(map.terrains[tnum].floor_fname, map.terrains[tnum].c_floor, map.terrains[tnum].floor_mode);
            } else {
                map.terrains[tnum].floor_texture = NULL;
            }
            printf(" floor_mode = %d, floor_fname = %s, c_floor=%d,%d,%d,%d\n", map.terrains[tnum].floor_mode, map.terrains[tnum].floor_fname, map.terrains[tnum].c_floor.r, map.terrains[tnum].c_floor.g, map.terrains[tnum].c_floor.b, map.terrains[tnum].c_floor.a);

        } else if (strcmp(&csv_line[offsets[0]],"#")==0) {
            // LINE = #,0,1,2,3,4,5,6,7,8,9,10,
            headers_done = 1;
        }
    }

    // The rest of the file is cell data...
    x=0; y=0;
    for (y=0;csv_line!=NULL;y++) {
        csv_line = load_line(file);
        if (csv_line==NULL) break;
        chomp(csv_line);   
        //printf("Split the line\n");
        split_csv_line_offsets();
        //printf("reading map y=%d\n",y);
        for (x=0;x<map.x_size;x++) {
            //cell = cells[x+1];
            if (strncmp(&csv_line[offsets[x+1]],"XX",2)==0){
                map.valid[x][y] = 0;
                //printf("%2d,%2d %s=> ??\n",x,y,&csv_line[offsets[x+1]]);
            } else {
                map.walls[x][y] = parse_hex(&csv_line[offsets[x+1]]);
                map.valid[x][y] = 1;
                //map.terrain[x][y] = (map.walls[x][y] >> 12) & 0xFF;
                //if (map.terrain[x][y]!=0) printf("terrain[%d][%d]=%d\n",x,y,map.terrain[x][y]);
                //printf("%2d,%2d %s=> %X\n",x,y,&csv_line[offsets[x+1]],map.valid[x][y]);
            }
            //printf("cell %s -> (%d)(%02x)\n",&csv_line[offsets[x+1]],map.valid[x][y], map.walls[x][y]);
        }
    };
    fclose(file);
    print_raw_map();
    //exit(0);
    return (1);
};

// SDL_Color to Integer
uint32_t ctoi (SDL_Color c) {
    uint32_t result;
    result = (c.r<<24) | (c.g<<16) | (c.b<<8) | c.a;
    return(result);
}

int write_map_csv (void) {
    FILE *file;
    int x,y; // current cell writing
    char filename[100];
    snprintf(filename,100,"Maps/map%d.out.csv",map.map_number);
    file = fopen(filename, "w"); // Open the file in write mode
    if (file == NULL) {
        printf("Could not open file %s\n", filename);
        return 1; // Exit if the file cannot be opened
    } else {
        printf("Opened %s for writing\n", filename);
    }
    
    // LINE 1: Map,1,Temple of Water
    fprintf(file,"Map,%d,\"%s\",\n",map.map_number,map.map_name);
    
    // LINE 2: Size,31,31
    fprintf(file,"Size,%d,%d,\n",map.x_size,map.y_size);

    if (map.map_number==1){
        fprintf(file,"Start,%0.2f,%0.2f,%d,\n",map.start_x,map.start_y,map.start_dir);
    }
    
    // LINES 3+: Up to 10 terrains, each with Name, Wall, Doors, and Floor colors and textures
    for (int tnum=0;tnum<10;tnum++) {
        if (map.terrains[tnum].NameT!=NULL) {
            fprintf(file,"NameT[%d],\"%s\",\n",tnum,map.terrains[tnum].NameT);
            fprintf(file,"WallT[%d],%d,%08X,\"%s\",\n",tnum,map.terrains[tnum].wall_mode,ctoi(map.terrains[tnum].c_wall), map.terrains[tnum].wall_fname);
            fprintf(file,"DoorT[%d],%d,%08X,\"%s\",\"%s\",\n",tnum,map.terrains[tnum].door_mode,ctoi(map.terrains[tnum].c_door), map.terrains[tnum].door_open_fname, map.terrains[tnum].door_closed_fname);
            fprintf(file,"FloorT[%d],%d,%08X,\"%s\",\n",tnum,map.terrains[tnum].floor_mode,ctoi(map.terrains[tnum].c_floor), map.terrains[tnum].floor_fname);
        }
    }
    
    // LINE: #,0,1,2,3,4,5,6,......
    fprintf(file,"# ");
    for (x=0;x<map.x_size;x++) {
        fprintf(file,",%4d",x);
    }
    fprintf(file,"\n");
    
    // LINES: 0,09,03,09,03,09,05,47,......
    for (y=0;y<map.y_size;y++) {
        fprintf(file,"%2d",y);
        for (x=0;x<map.x_size;x++) {
            if (map.valid[x][y]==0) {
                fprintf(file,",XXXX");
            } else {
                fprintf(file,",%04X",map.walls[x][y]);
            };
        };
        fprintf(file,"\n");
    }
    fclose(file);
    return(0);
}

int read_objects_csv (char *filename) {
    FILE *file;
    char * cell;
    //char * line;
    //int uid_owner_num;
    //int owner_mapnumber;
    int owner_object_num;   // current spot relative to owner
    //struct object_s *owner; // list that will contain the object
    struct object_s obj; // current object being built
    char **cells = malloc(sizeof(char*) * 50); // Ensure MAX_CELLS is defined
    if (cells == NULL) {
        printf("Memory allocation failed for cells\n");
        return 1; // Handle error
    }
    //owner=objects;
    owner_object_num = 0;
    file = fopen(filename, "r"); // Open the file in read mode
    if (file == NULL) {
        printf("Could not open file %s\n", filename);
        return 1; // Exit if the file cannot be opened
    } else {
        printf("Opened file %s\n", filename);
    }
    csv_line = load_line(file); // throw away first line of comments
    chomp(csv_line);   
    while (csv_line!=NULL) {
        //printf("loading line\n");
        csv_line = load_line(file);
        //printf("done loading line ptr=%p\n",csv_line);
        if (csv_line==NULL) break;
        chomp(csv_line);
        printf("read_objects_csv:%s\n",csv_line);
        if(csv_line[0]=='#') {
            //printf("comment: '%c'\n",csv_line[0]);
            continue;  // skip comment lines
        } else {
            //printf("not comment: '%c'\n",csv_line[0]);
        }
        // split the line
        //printf("Split the line\n");
        //int num_cells = split_csv_line(line,cells,400);
        //split_csv_line(line,cells,400);
        split_csv_line_offsets();
        //printf("num_cells %d\n",num_cells);
        //----- CSV_OWNER -----
        // identify parent container
        cell=&csv_line[offsets[CSV_OWNER]];
        obj.owner = cell[0];

        //----- CSV_OWNER_NUM -----
        cell=&csv_line[offsets[CSV_OWNER_NUM]];
        obj.owner_num = atoi(cell);

        //printf("CSV_OWNER = %c, owner_object_num=%d\n",obj.owner, obj.owner_num);
        //----- CSV_UID -----
        cell=&csv_line[offsets[CSV_UID]];
        obj.uid = atoi(cell);
        //printf("*** UID = %d\n",obj.uid);

        //----- CSV_LOCX -----
        cell=&csv_line[offsets[CSV_LOCX]];
        obj.x = atof(cell);

        //----- CSV_LOCY -----
        cell=&csv_line[offsets[CSV_LOCY]];
        obj.y = atof(cell);
        //printf("*** LOC = %f,%f\n",obj.x, obj.y);

        //----- CSV_DIR -----
        cell=&csv_line[offsets[CSV_DIR]];
        obj.dir = atoi(cell);
        //printf("*** DIR = %d\n",obj.dir);

        //----- CSV_FEATURE -----
        cell=&csv_line[offsets[CSV_FEATURE]];
        if (strcasecmp(cell,"FEATURE")==0) {
            obj.permanent = 1;
        } else {
            obj.permanent = 0;
        }
        //printf("*** FEATURE = %s, (permanent = %d)\n",cell,obj.permanent);

        //----- CSV_TYPE -----
        cell=&csv_line[offsets[CSV_TYPE]];
        obj.common_type = cell[0];
        //printf("*** TYPE = %c\n",obj.common_type);

        //----- CSV_NAME -----
        cell=&csv_line[offsets[CSV_NAME]];
        obj.name = str_alloc_copy(cell);
        //printf("*** NAME = %s\n",obj.name);

        //----- CSV_DESCRIPTION -----
        cell=&csv_line[offsets[CSV_DESCRIPTION]];
        obj.description = str_alloc_copy(cell);
        //printf("*** DESCRIPTION = %s\n",obj.description);

        //----- CSV_GRAPHIC_NAME -----
        cell=&csv_line[offsets[CSV_GRAPHIC_NAME]];
        obj.graphic_name = str_alloc_copy(cell);
        obj.graphic_type = 0;
        //printf("*** GRAPHIC_NAME = %s\n",obj.graphic_name);

        //----- CSV_MODE -----
        cell=&csv_line[offsets[CSV_MODE]];
        obj.object_mode = atoi(cell);
        //printf("*** SIZE = %0.2f\n",obj.size);

        //----- CSV_COLOR_RGBA -----
        cell=&csv_line[offsets[CSV_COLOR_RGBA]];
        uint32_t graphic_color_hex = parse_hex(cell);
        printf("*** COLOR = %08X\n",graphic_color_hex);
        obj.graphic_color.r = (graphic_color_hex & 0xFF000000) >> 24;
        obj.graphic_color.g = (graphic_color_hex & 0x00FF0000) >> 16;
        obj.graphic_color.b = (graphic_color_hex & 0x0000FF00) >>  8;
        obj.graphic_color.a = (graphic_color_hex & 0x000000FF);

        //----- CSV_SIZE -----
        cell=&csv_line[offsets[CSV_SIZE]];
        obj.size = atof(cell);
        //printf("*** SIZE = %0.2f\n",obj.size);

        //----- CSV_KEEPOUT -----
        cell=&csv_line[offsets[CSV_KEEPOUT]];
        obj.keepout = atof(cell);
        //printf("*** SIZE = %0.2f\n",obj.size);

        //----- CSV_QUANTITY -----
        cell=&csv_line[offsets[CSV_QUANTITY]];
        obj.quantity = atoi(cell);
        //printf("*** QUANTITY = %d\n",obj.quantity);

        //----- CSV_VALID -----
        cell=&csv_line[offsets[CSV_VALID]];
        obj.valid = atoi(cell);
        //printf("*** VALID = %d\n",obj.valid);

        //----- CSV_BUTTON_TEXT -----
        cell=&csv_line[offsets[CSV_BUTTON_TEXT]];
        obj.button_text = str_alloc_copy(cell);
        //printf("*** BUTTON_TEXT = %s\n",obj.button_text);

        //----- CSV_CUSTOM_DATA -----
        cell=&csv_line[offsets[CSV_CUSTOM_DATA]];
        obj.custom_data = str_alloc_copy(cell);
        //printf("*** CUSTOM_DATA = %s\n",obj.custom_data);

        //----- CSV_COMMENT -----
        cell=&csv_line[offsets[CSV_COMMENT]];
        obj.comment = str_alloc_copy(cell);
        //printf("*** COMMNENT = %s\n",obj.comment);

        // ----- Other Object Fields -----
        obj.texture = get_frect_by_name(obj.graphic_name,
                                        &obj.frect_count,
                                        &obj.graphic_type);

        // add obj to container
        //printf("owner[%d] at %p\n",owner_object_num, &owner[owner_object_num]);
        memcpy(&objects[owner_object_num],&obj,sizeof (struct object_s));
        //printf("memcpy complete\n");
        num_objects++;
        owner_object_num++;
        // read texture
        // free memory
    }
    fclose(file);
    //printf("return 1\n");
    return (1);
}

int write_objects_csv (char *filename) {
    FILE *file;
    int i;

    file = fopen(filename, "w"); // Open the file in write mode
    if (file == NULL) {
        printf("Could not open file %s\n", filename);
        return 1; // Exit if the file cannot be opened
    } else {
        printf("Opened file %s\n", filename);
    }
    //----- line 1: Header -----
    fprintf(file,"#Owner,Owner_num,UID,Loc_X,Loc_Y,dir,FEATURE/OBJECT(permanent),type,name,description,graphic_name,size,keepout,quantity,valid,Button,Custom-Data,Comment\n");

    for (i=0;i< num_objects; i++) {
        //----- CSV_OWNER -----
        fprintf(file,"%c,",objects[i].owner);

        //----- CSV_OWNER_NUM -----
        fprintf(file,"%d,",objects[i].owner_num);

        //----- CSV_UID -----
        fprintf(file,"%d,",objects[i].uid);
        //printf("*** UID = %d\n",objects[i].uid);

        //----- CSV_LOCX -----
        fprintf(file,"%0.2f,",objects[i].x);

        //----- CSV_LOCY -----
        fprintf(file,"%0.2f,",objects[i].y);
        //printf("*** LOC = %0.2f,%0.2f\n",objects[i].x, objects[i].y);

        //----- CSV_DIR -----
        fprintf(file,"%d,",objects[i].dir);
        //printf("*** DIR = %d\n",objects[i].dir);

        //----- CSV_FEATURE -----
        if (objects[i].permanent==1) {
            fprintf(file,"FEATURE,");
        } else {
            fprintf(file,"OBJECT,");
        }
        //printf("*** PERMANENT = %d)\n",objects[i].permanent);

        //----- CSV_TYPE -----
        fprintf(file,"%c,",objects[i].common_type);
        //printf("*** TYPE = %c\n",objects[i].common_type);

        //----- CSV_NAME -----
        fprintf(file,"%s,",objects[i].name);
        //printf("*** NAME = %s\n",objects[i].name);

        //----- CSV_DESCRIPTION -----
        fprintf(file,"\"%s\",",objects[i].description);
        //printf("*** DESCRIPTION = %s\n",objects[i].description);

        //----- CSV_GRAPHIC_NAME -----
        fprintf(file,"\"%s\",",objects[i].graphic_name);
        //printf("*** GRAPHIC_NAME = %s\n",objects[i].graphic_name);

        //----- CSV_SIZE -----
        fprintf(file,"%0.2f,",objects[i].size);
        //printf("*** SIZE = %f\n",objects[i].size);

        //----- CSV_KEEPOUT -----
        if (objects[i].keepout !=0.0) {
            fprintf(file,"%0.2f,",objects[i].keepout);
        } else {
            fprintf(file,",");
        }
        //printf("*** KEEPOUT = %f\n",objects[i].keepout);

        //----- CSV_QUANTITY -----
        fprintf(file,"%d,",objects[i].quantity);
        //printf("*** QUANTITY = %d\n",objects[i].quantity);

        //----- CSV_VALID -----
        fprintf(file,"%d,",objects[i].valid);
        //printf("*** VALID = %d\n",objects[i].valid);

        //----- CSV_CUSTOM_DATA -----
        fprintf(file,"\"%s\",",objects[i].button_text);
        //printf("*** CUSTOM_DATA = %s\n",objects[i].custom_data);

        //----- CSV_CUSTOM_DATA -----
        fprintf(file,"\"%s\",",objects[i].custom_data);
        //printf("*** CUSTOM_DATA = %s\n",objects[i].custom_data);

        //----- CSV_COMMENT -----
        fprintf(file,"\"%s\"\n",objects[i].comment);
        //printf("*** COMMENT = %s\n",objects[i].comment);

        //objects[i].num_objects = 0; // initialize the new object
    }
    fclose(file);
    //printf("return 1\n");
    return (1);
}

// copies a string to a newly malloc'd pointer
char *str_alloc_copy(char *src) {
    char *dest;
    dest = (char *)malloc(sizeof(char) * (strlen(src)+1));
    if (dest==NULL) {
        printf("malloc error in str_alloc_cpy()");
        exit(-1);
    }
    strcpy(dest, src);
    return dest;
}

// copies a string to a newly malloc'd pointer
// but first frees up an old alloc'd pointer.
char *str_realloc_copy(char *src, char *old_dest) {
    char *dest;
    if (old_dest!=NULL) free(old_dest);
    dest = (char *)malloc(sizeof(char) * (strlen(src)+1));
    if (dest==NULL) {
        printf("malloc error in str_realloc_cpy()");
        exit(-1);
    }
    strcpy(dest, src);
    return dest;
}

// splits a CSV line into substrings, allows quoted cells,
// and eliminates spaces at beginning and end of cell text.
// Does so by modifying the input string (GLOBAL cvs_line)
// replacing commas with \0, and returning offsets to the
// beginning of each substring.
// if you want to save a substring, then copy it out before
// loading a new line into the buffer.
void split_csv_line_offsets(void) {
    int i = 0;      // position in line we are working on
    //int j = 0;      // a different loop counter
    int quoted = 0; // flag indicating we are in the middle of a quoted string
    int begun = 0;  // flag indicating first non-space character not yet encountered
    int linelen;    // length of original line
    csv_num_cells = 0;          // GLOBAL
    linelen = (int)strlen(csv_line); // we will modify the original, so remember the full length
    offsets[(int)csv_num_cells] = i; // GLOBAL
    csv_num_cells++;            // GLOBAL
    for(i=0;i<=linelen;i++) {
        if ((begun==0) && (isspace(csv_line[i]))) {
            offsets[csv_num_cells-1]+=1;
            //printf("Eating leading space in %d\n",csv_num_cells-1);
            continue;
        }
        if (csv_line[i]=='"') {
            quoted = !quoted; 
            if (begun==0) {
                offsets[csv_num_cells-1]+=1;
            }
        }
        begun = 1;
        if (((csv_line[i]==',') && (!quoted)) || csv_line[i]=='\0') {
            csv_line[i] = '\0';
            for (int xx=i-1;csv_line[xx]=='"' || isspace(csv_line[xx]);xx--) {
                csv_line[xx]='\0';
                //printf("Eating trailing space in %d\n",csv_num_cells-1);
            }
            offsets[(int)csv_num_cells] = i+1;
            csv_num_cells += 1;
            begun=0;
        }
    }
    //for (j=0;j<csv_num_cells;j++) {
        //printf("%d = >%s<\n",j,&csv_line[offsets[j]]);
    //}
}

// print_raw_map: display raw hex data for entire map
int print_raw_map(void) {
    int x,y;
    printf("MAP-NAME = %s\n", map.map_name);
    printf("MAP-DIMENSIONS = %d x %d\n", map.x_size, map.y_size);
    if (strlen(map.terrains[0].wall_fname)>1) {
        printf("MAP-WALLS = %s\n",map.terrains[0].wall_fname);
    }
    for (y=0; y<map.y_size; y++) {
        for (x=0; x<map.x_size; x++) {
            if (map.valid[x][y]) {
                printf(" %02X", map.walls[x][y]);
            } else {
                printf(" XX");
            }
        }
        printf("\n");
    }
    return(0);
}

void chomp(char *str) {
    size_t len;
    //printf("Chomping %s\n",str);
    if (str == NULL) {return;}
    len = strlen(str);
    if (len > 0 && str[len - 1] == '\n') {
        str[len - 1] = '\0'; // Replace newline with null terminator
        //printf("chomped: %s\n", str);
    }
    // in DOS environment, also get rid of carriage return which preceeds newline
    if (len > 1 && str[len - 2] == '\r') {
        str[len - 2] = '\0'; // Replace carriage return with null terminator
        //printf("chomped: %s\n", str);
    }
}

uint32_t parse_hex(char *hex_str) {
    char *endptr; // Pointer to track the end of the parsed string
    long int value = strtol(hex_str, &endptr, 16); // Convert hex string to long int
    // Check for errors: if no digits were found or if the entire string was not parsed
    if (endptr == hex_str) {
        //printf("  No digits were found in the input string.\n");
        return 0; // Indicate an error (or zero value)
    }
    if (*endptr != '\0') {
        //printf("  Extra characters found after the number: %s\n", endptr);
    }
    return (int)value; // Return the parsed value as an integer
}

// line is returned in a dynamically allocated buffer.  free() this memory when done.
char *load_line(FILE *file) {
    //char ch;
    //static char line[400];
    ////size_t len = 400; // maximum length of line
    static char *line;
    size_t len; // maximum length of line
    ssize_t read;

//    line = (char *)malloc(len + 1); // Allocate memory for the substring
//    if (line == NULL) {
//        printf("load_line: malloc(%zu) failed\n",len+1);
//        return NULL; // Return NULL if memory allocation fails
//    }
    if (line != NULL) { // cleaneup after a previous call.
        free (line);
        line = NULL;
        len = 0;
    }
    
    if ((read = getline(&line, &len, file)) != -1) {
        //printf("load_line: Retrieved line of length %zd\n", read);
        //printf("%s\n", line);
        return line;
    };
    return NULL;
}

void print_all_detailed_objects(void) {
    printf("Owner Valid UID  X.XX, Y.YY Dir Size Type    Name         Description GType & Gname                 Perm Held Qty Button -&- Misc  -&- Comment\n");
    printf("----- ----- --- ----- ----- --- ---- ---- ------- ------------------- ----------------------------- ---- ---- --- ------ -&- ----- -&- -------\n");
    for (int i=0; i<num_objects; i++) {
        print_detailed_object(&objects[i]);
    }
}

// debug function
void print_detailed_object(struct object_s *obj) {
        printf("%c ",      obj->owner);
        printf("%2d  ",       obj->owner_num);
        printf("%1d    ",   obj->valid);
        printf("%2d   ",    obj->uid);
        printf("% 1.2f,% 1.2f ", obj->x, obj->y);
        printf("%1d   ",    obj->dir);
        printf("%1.2f ",    obj->size);
        printf("%1c ",      obj->common_type);
        printf("%10s ",     obj->name);
        printf("%19s ",     obj->description);
        printf("%1d ",      obj->graphic_type);
        printf("%27s ",     obj->graphic_name);
        printf("%1d    ",   obj->permanent);
        printf("%1d    ",   obj->held);
        printf("%2d  ",     obj->quantity);
        printf("%4s -&- ",  obj->button_text);
        printf("%4s -&- ",  obj->custom_data);
        printf("%1s ",      obj->comment);
        printf("\n");
}
