// monster.h

struct monster_s {
    char   *name;
    int     uid;
    char    common_type;
    char   *description;
    int     graphic_type;
    //int     mode;       // -1=no_textuire, 0=no blend, 1=blend, 2=add, 4=mod, 8=multiply
    char   *graphic_name;
    void   *texture;
    int     monster_mode;
    int     frect_count;
    SDL_Color graphic_color;
    float   size;
    int     valid;
    char   *comment;
    char   *custom_data;
    
    float   x;                // current location across map
    float   y;                // current location down map
    int     dir;              // current direction
    int     map_number;       // current map
    int     route_following;    //
    float   target_x;         // Target location for route planning
    float   target_y;
    float   step_size;        // TODO: Rename speed

    int     health;
    int     damage;
    char   *weapon_name;
    int     armor_class;
    int     quantity;

    unsigned char target_map[68][68]; // Distance map from route planning
    unsigned char known_map[68][68];  // visible and discovered area of current map

    int     option_open_doors;  // will auto-open a door if you try to walk through it
    int     option_close_doors; // will auto-close a door after you walk through it
    int     option_avoid_walls; // will walk away from a wall when too close.
    int     option_graphic_dir; // whether graphic rotates to direction.
};

// GLOBAL variables
extern struct monster_s monsters[20];    // Player's current x,y,dir
extern int num_monsters;

extern double dummyd; // temporary necessary for modf, unused value
//extern int player_following;
//extern float player_step_size;

// Simplified floating point math functions:
// round float to nearest integer
#define iround(f) ((int)round(f))
// round down float to nearest integer
#define ifloor(f) ((int)floor(f))
// round up float to next nearest integer
#define iceil(f) ((int)ceil(f))
// get the fractional part of float
#define frac(f) ((float)modf(f,&dummyd))

// Function prototypes
extern int imin(int a, int b);
extern int imax(int a, int b);

extern int load_monsters(char *filename);
extern void monster_create(int mnum);


// called in main, map_io::read_map, user_input::handle_soft_ui_button_event
extern void monster_move_to(int mnum, float x,float y, int dir);   // Initialize, or teleport

// called in user_input::handle_user_events, user_input:: handle_keyboard_events
extern int monster_follow_route(int mnum);
  extern void monster_move(int mnum, int dir, float distance);      // Move the player
    extern float monster_can_move(int mnum, float x, float y, int dir, float distance); // How far can player move?
  extern int monster_open(int mnum, int dir);                       // Open a door
    extern int monster_can_open(int mnum, float x, float y, int dir);                   // Is there a door to open?

// called in user_input::handle_mouse_event
extern int monster_plan_route(int mnum, float route_to_x, float route_to_y, int use_doors, int use_known_space);

extern int monster_view_simple(int mnum);

extern int monster_visible_to_player(int mnum);
extern int player_visible_to_monster(int mnum);
extern void monster_random_walk(int mnum);
