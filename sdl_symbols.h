// sdl_gui.h

// GLOBAL variables

// Function Prototypes
// called by main
extern int load_textures(void); // loads texture for each object on map
  //extern SDL_Texture* load_symbol_texture( char *path, int r, int g, int b, int a ); // loads a single texture
  extern SDL_Texture* load_symbol_texture( char *path, SDL_Color color, int mode ); // loads a single texture

// called by sdl_draw::sdl_draw_objects
extern int draw_image (SDL_Texture *texture, int x,int y,int w,int h,int dir,int panel); // renders a texture