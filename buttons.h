// buttons.h

struct button_s {
    char *text;             // text in the button
    SDL_Rect rect;          // screen area button occupies
    SDL_Texture *texture;   // pre-rendered button
    int visible;            // boolean, whether to draw it
    int valid;              // boolean, whether texture is valid
    int object_uid;         // ID of the associated object
    SDL_Color fg_color;     // color of foreground
    SDL_Color bg_color;     // color of background (includes alpha)
    SDL_Color border_color; // color of frame around text
};

// gloval variables
extern TTF_Font *font;

extern struct button_s button_list[100]; // list of buttons
extern int num_buttons;                 // number of buttons

// LOCAL Globals
//extern int button_font_size;            // global font size for buttons
//extern int button_border_size;          // pixels added to text each side
//extern int button_spacing;              // space between two buttons
//extern int buttons_from_edge_spacing;   // Don't draw buttons right at edge

// Function Prototypes

// called by sdl_draw::sdl_start
extern void start_ttf(void);    // library and font setup
// called by sdl_draw::sdl_quit
extern void quit_ttf(void);     // library and font destroy

// called by main
extern void create_all_object_buttons(void);    // creates a button to match each object
  extern int button_create(char *text, int uid); // creation
    extern SDL_Color color_from_rgba(int r, int g, int b, int a);// helper function
    extern SDL_Texture *button_create_texture(int button_num);   // texture is only the TEXT, not frame/background

// three unused functions
//extern int button_hide(int button_num);         // visible = 0;
//extern int button_show(int button_num);         // visible = 1;
//extern int button_destroy(int button_num);      // valid = 0; destroy_texture

// called by user_input::handle_mouse_event
extern int which_button(int scr_x, int scr_y);   // converts screen coordinates to button number

// unused
//extern int find_buttons_by_uid (int uid, int *matching_buttons);    // looks for object UID, returns button number list

// called by sdl_draw::dirty_display_handler
extern int display_buttons_for_objects(void);   // part of main draw routine
  extern int enable_disable_buttons_by_uid (int obj_uid, int enable); // converts text label to button number

// called by sdl_draw::dirty_display_handler
//extern int arrange_and_draw_UpperLeft_buttons(void); // unused, old
extern int arrange_and_draw_buttons(void);        // arrange and draw all buttons
  extern int draw_one_button(int button_num);     // render a single button

