// dungeon.h
// This file contains global structs and functions relating to maps.

// Floating point Rectangle (equivalent to SDL_FRect)
struct F_Rect_s {
    float x;
    float y;
    float w;
    float h;
};

// Floating point Point (equivalent to SDL_FPoint)
struct F_Point_s {
    float x;
    float y;
};

typedef uint8_t byte;

// colorize texture
struct color_texture_mode_s {
    byte texture;
    byte texture_colored;
    byte line_colored;
    byte rectange_colored;
};

//typedef struct SDL_Color
//{
//    Uint8 r;
//    Uint8 g;
//    Uint8 b;
//    Uint8 a;
//} SDL_Color;

/*
struct F_Vector_s {
    SDL_FPoint position;        // Vertex position, in SDL_Renderer coordinates
    SDL_Color  color;           // Vertex color
    SDL_FPoint tex_coord;       // Normalized texture coordinates, if needed
};
*/

// objects_s: a movable object in the game.
struct object_s {
    char  owner;            // who owns this (eg, 'M'ap, 'U'ID, 'P'layer, 'M'onster)
    int   owner_num;        // the number of the owner above
    int   uid;              // unique ID number (or 0 if don't care)
    char  common_type;      // simple type of objecct (W=weapon)
    char *name;             // short name (sword+1)
    char *description;      // long description (sword +1 with a jeweled hilt that glows blue)

    int   graphic_type;     // 1=texture, 2=F_Rect, 3=F_Points, 0=none
    int   blend_mode;       // -1=no_textuire, 0=no blend, 1=blend, 2=add, 4=mod, 8=multiply
    char *graphic_name;     // texture filename (PNG/relic-sword.png)
    char graphic_color_mode;
    SDL_Color graphic_color;
    int   object_mode;
    void *texture;          // SDL_Texture* or F_Rect* or F_Point*
    int   frect_count;      // count of number of frect rectangles or fpoint points

    float x;                // map coordinates (5.0, 5.25 is slightly south of center 5,5
    float y;                // map coordinates
    float size;             // size of object rectangle 1.0=full tile.
    float keepout;          // diameter of keepout area around feature. 1.0=full tile
    byte  dir;              // rotate (0=N/1=E/2=S/3=W)
    byte  permanent;        // not movable flag
    byte  held;             // in inventory flag
    int   quantity;         // for gold, arrows, etc.
    byte  valid;            // if 0, item likely destroyed or used up.
    char *button_text;      // text for the object's button when nearyby on map
    char *custom_data;      // various data dependent on type of object.
    char *comment;          // helpful column in data file for notes.
}; // objects[] instantiated in dungeon

// One entry describes what a cell looks like with various textures.
struct terrain_s {
    char *NameT;         // name that terrain
    // Walls
    int   wall_mode;
    char *wall_fname;    // filename for walls texture
    SDL_Texture *wall_texture; // the walls texture
    SDL_Color c_wall;     // wall color (when visible)
    
    // Doors
    int door_mode;
    char *door_open_fname;
    char *door_closed_fname;
    SDL_Texture *door_open_texture;
    SDL_Texture *door_closed_texture;
    SDL_Color c_door;     // door color (when visible)
    
    // Floors
    int floor_mode;
    char *floor_fname;
    SDL_Texture *floor_texture;
    SDL_Color c_floor;    // floor color (when visible)
    
    int view_cost;        // limits how far can be seen
    int travel_cost;      // limits how far can be seen
};

// everything to know about the map
struct map_s {
    char map_fname[20];   // filename for map (Maps/map1.csv)
    int  map_number;      // 'floor' number foor this map
    char *map_name;       // name for display (Crypt of Bob)

    struct terrain_s terrains[10];

    int view_distance;    // how far to see on this map
    int travel_distance;  // how far to move per step on this map
    SDL_Color c_fog;      // fog modifier for known areas of map
    SDL_Color c_unk;      // unknown area color

    int x_size;           // number of map tiles across (32)
    int y_size;           // number of map tiles down   (32)

    float start_x;          //
    float start_y;          //
    int   start_dir;        //

    u_int32_t walls[68][68];    // walls and doors and invisible walls in bitmap format, per tile.
                                // [3:0] = walls per direction
                                // [7:4] = doors per direction
                                // [11:8]  = invisible walls per direction
                                // [16:12] = terrain style index to list terrains
    byte visible[68][68]; // player map visibility: 0=unk, 1=known(fog), 2=visible
    byte valid[68][68];   // flag for valid/invalid tiles

};  // map and maps[] : Global instantiated in dungeon.c

struct tile_s {
    uint32_t       walls :4;      // drawm walls
    uint32_t       doors :4;      // doors can be opened
    uint32_t       keepout :4;    // invisible walls
    uint32_t       spare_byte :4;
    uint32_t       floor_c :4;    // color of floor
    uint32_t       wall_c :4;     // color of walls
    uint32_t       door_c :4;     // color of doors
    uint32_t       map_vis :1;    // visible tile
    uint32_t       map_known :1;  // known map tile
    uint32_t       valid :1;      // valid part of map
    uint32_t       spare_bit :1;
};

union tile_u {
    uint32_t      all;
    struct tile_s field;
};

// GLOBAL variables
extern struct map_s map;        // the current map
extern struct map_s maps[10];   // all the maps
extern int valid_maps[10];      // is each map already loaded?
extern int current_map_num;
extern struct object_s objects[50];
extern int num_objects;

#define ifloor(f) ((int)floor(f))
// Function prototypes
// called by sdl_draw:: and player:: and dungeon::
extern int get_wall(int number, int position);
extern int get_wall_xy(float x, float y, int dir);
extern int get_door(int number, int position);       // returns bit[4] whether door is present in dir
extern int get_door_xy(float x, float y, int dir);
extern int get_keepout(int number, int dir);
extern int get_keepout_xy(float x, float y, int dir);
extern int get_both_bits(int number, int position);  // OR of both get_wall and get_door
extern int get_both_bits_xy(float x, float y, int dir);
extern int toggle_wall_xy(float x, float y, int dir, int offset);
extern int get_any_wall(int number, int position);   // invisible walls count as walls
extern int get_any_wall_xy(float x, float y, int dir);
extern int opposite(int dir);                        // opposite direction from dir
extern int rotate_r(int dir);                        // right rotation from dir
extern int rotate_l(int dir);                        // left rotation from dir
extern int get_terrain_xy(int x, int y);
extern int get_view_cost_xy(int x, int y);
extern int get_travel_cost_xy(int x, int y);
extern int get_view_distance(void);
extern int get_travel_distance(void);
// unused
extern float dir_to_degrees(int dir);

// called by sdl_draw::dirty_display_handler
extern int player_view_simple(void);                 // fills map.visible with current visible room/halls
extern int player_view_noback(void);
// TODO: add more complex player view functions
