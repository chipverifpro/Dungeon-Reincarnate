// player.h

struct player_s {
    float   x;                // current location across map
    float   y;                // current location down map
    float   dir;              // current direction
    int     map_number;       // current map
    float   target_x;         // Target location for route planning
    float   target_y;
    float   step_size;        // was player_step_size

    int     health;
    int     armor_class;
    int     damage;
    char   *weapon_name;
    int     ranged_attack;
    int     spell_attack_damage;
    int     magic_points;
    char   *spell_name;

    unsigned char target_map[68][68]; // Distance map from route planning
    // player's known_map is in the map_s structure

    int     route_following;    
    int     option_open_doors;  // will auto-open a door if you try to walk through it
    int     option_close_doors; // will auto-close a door after you walk through it
    int     option_avoid_walls; // will walk away from a wall when too close.
};

// GLOBAL variables
extern struct player_s pl;    // Player's current x,y,dir

extern double dummyd; // temporary necessary for modf, unused value

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

// called in main
void player_init(void);

// called in main, map_io::read_map, user_input::handle_soft_ui_button_event
extern void player_move_to(float x,float y, int dir);   // Initialize, or teleport

// called in user_input::handle_user_events, user_input:: handle_keyboard_events
extern int player_follow_route(void);
  extern void player_move(int dir, float distance);      // Move the player
    extern float player_can_move(float x, float y, int dir, float distance); // How far can player move?
  extern int player_open(int dir);                       // Open a door
    extern int player_can_open(float x, float y, int dir);                   // Is there a door to open?

// called in items::player_get_item/player_open_chest, buttons::display_buttons_for_objects
extern float get_distance(float x1,float y1, float x2,float y2); // how far are two points from each other?
//extern int player_get_item(float x, float y);          // Grab an item that is in reach

extern void print_distancemap(unsigned char distance_map[68][68], int minx,int miny,int maxx,int maxy);

// called in user_input::handle_mouse_event
extern int plan_route(unsigned char distance_map[68][68], float route_to_x, float route_to_y, int use_doors, int use_known_space);

