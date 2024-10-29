// sdl_gui.h

// GLOBAL variables
extern int game_time;       // number of animation ticks since run started.
extern int animation_time;  // ms between animation callback.

// Function Prototypes

// called by system timer
extern Uint32 callback(Uint32 interval, void* name); // updates game_time
  extern void send_user_event(int event_code);   // send a user event to the main processing loop

// called by sdl_draw::sdl_start
extern int setup_callback_interval (int interval_ms);
// called by sdl_draw::sdl_quit
extern int remove_callback_interval (void);

// called by user_input::main_user_interface_loop
extern void print_statistics(void);
