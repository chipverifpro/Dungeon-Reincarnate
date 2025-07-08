// sdl_gui.h
// this file contains globals, constants, and function prototypes
// for SDL2 user interface functions (process inputs, draw outputs).

//GLOBAL variables
extern int dirty_display; // when 0, no redraw is necessary.
extern float map_origin_x, map_origin_y; // in screen coordinates
extern float sw_scale; // allows zooming in/out of map
extern int screen_width;
extern int screen_height;
extern int mode_inventory;  // shift is held down

//GLOBAL variables only for modules using SDL
extern SDL_Window *window;     // for global graphics context
extern SDL_Renderer *renderer; // for global graphics context
extern SDL_Event event;        // current keyboard/mouse event
extern SDL_Texture *walls_texture[16];
extern int num_walls_texture;
extern SDL_Texture *texture;   // NOT the screen, but a separate drawing target.

// Default starting parameters
// window dimensions
#define SCREEN_WIDTH 600
#define SCREEN_HEIGHT 600
// allows blockier drawing using hardware scaling
#define HW_SCALE 1
// default number of pixels per tile (software scaling)
#define SW_SCALE 60
// wall thickness
#define WT 0.05

// macros for selecting R, G, B, Alpha
#define COLOR_WHITE   0xFF, 0xFF, 0xFF, 0xFF
#define COLOR_GREY_L  0xC0, 0xC0, 0xC0, 0xFF
#define COLOR_GREY_M  0x80, 0x80, 0x80, 0xFF
#define COLOR_GREY_D  0x40, 0x40, 0x40, 0xFF
#define COLOR_BLACK   0x00, 0x00, 0x00, 0xFF
#define COLOR_RED     0xFF, 0x00, 0x00, 0xFF
#define COLOR_GREEN   0x00, 0xFF, 0x00, 0xFF
#define COLOR_BLUE    0x00, 0x00, 0xFF, 0xFF
#define COLOR_PURPLE  0x80, 0x00, 0x80, 0xFF
#define COLOR_TEAL    0x00, 0x80, 0x80, 0xFF
#define COLOR_YELLOW  0xFF, 0xFF, 0x00, 0xFF
#define COLOR_CYAN    0x00, 0xFF, 0xFF, 0xFF
#define COLOR_ORANGE  0xFF, 0x80, 0x00, 0xFF
#define COLOR_BROWN   0x85, 0x2A, 0x2A, 0xFF
#define COLOR_TRANSPARENT       0x00, 0x00, 0x00, 0x00
#define COLOR_BLACK_TRANSPARENT 0x00, 0x00, 0x00, 0x7F
#define COLOR_WHITE_TRANSPARENT 0xFF, 0xFF, 0xFF, 0x7F
#define COLOR_NONE    -1, -1, -1, -1


// Function Prototypes

// called by map_io::
void *get_frect_by_name(char *frect_name, int *count, int *is_rects); // looks up F_Rect or F_Point by name

// main functions to transform a rectange based symbol into screen coordinates
// called by sdl_draw::sdl_draw_map
extern void sdl_draw_transformed_rects(struct F_Rect_s *rects, int rcount, int dir, float tile_x, float tile_y, float size_x, float size_y);
  extern struct F_Rect_s *rotate_rects(struct F_Rect_s *src, int num, int dir);
  extern struct F_Rect_s *scale_to_center_rects(struct F_Rect_s *src, int num, float sx, float sy);
  extern struct F_Rect_s *scale_rects(struct F_Rect_s *src, int num, float sx, float sy);
  extern struct F_Rect_s *move_rects(struct F_Rect_s *src, int num, int dx, int dy);
  extern SDL_Rect *convert_rects(struct F_Rect_s *src, int num);

// main functions to transform a point based symbol into screen coordinates
// called by sdl_draw::sdl_draw_map/draw_shadows
extern void sdl_draw_transformed_lines(struct F_Point_s *symbol_points, int pcount, int dir, float tile_x, float tile_y, float size_x, float size_y);
  extern struct F_Point_s *rotate_points(struct F_Point_s *src, int num, int dir);
  extern struct F_Point_s *scale_to_center_points(struct F_Point_s *src, int num, float sx, float sy);
  extern struct F_Point_s *scale_points(struct F_Point_s *src, int num, float sx, float sy);
  extern struct F_Point_s *move_points(struct F_Point_s *src, int num, int dx, int dy);
  extern SDL_Point *convert_points(struct F_Point_s *src, int num);

// called by sdl_draw: draw_shadows
extern void sdl_draw_transformed_polygon(struct F_Point_s *symbol_points, int pcount, int dir, float tile_x, float tile_y, float size_x, float size_y);
  extern SDL_Vertex *convert_scr_points_to_vertexes(SDL_Point *src, int num,
                    uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha);
  SDL_Vertex *clip_vertexes(SDL_Vertex *src, int *num);

// unused
extern void test_clipper(void);

// called by user_input::handle_mouse_event
extern struct F_Point_s convert_scr_to_map(int scr_x, int scr_y);

// helper functions
extern struct F_Rect_s make_F_Rect(float x, float y, float size_x, float size_y);
extern struct F_Point_s make_F_Point(float x, float y);
//unused
extern void print_frects(struct F_Rect_s *rlist, int rcnt); // debug
extern void print_srects(SDL_Rect *rlist, int rcnt);        // debug

// 
// creat a texture to store the known map
// called by user_input.c::main_loop
extern void dirty_display_handler(void); // render display if dirty flag set.
  extern int sdl_draw_view(float x_center, float y_center, int style); // main draw function
    extern int sdl_draw_map(float x_center, float y_center, int style);     // draw map walls
    extern int sdl_draw_objects(float x_center, float y_center, int style); // draw map objects
    extern int sdl_draw_monsters(float x_center, float y_center, int style); // draw map objects
    extern int sdl_draw_player(float x_center, float y_center, int style);  // draw player
    extern int sdl_draw_shadows(float x_center, float y_center);         // draws shadows from visible walls
  // for creating a texture that will be used by draw_shadows
  extern SDL_Texture *create_render_texture (int size_x, int size_y);
  extern int set_render_to_texture (SDL_Texture *texture_target, int clear,
                    uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha);

// called by main
extern int sdl_start(void);                 // initializes SDL2
extern int sdl_quit(void);                  // cleans and deallocates SDL2

extern int SetRenderDrawColorPct(SDL_Renderer *renderer, SDL_Color c, float pct);
