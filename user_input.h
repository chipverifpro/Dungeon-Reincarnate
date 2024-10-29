// user_input.h
// this file contains globals, constants, and function prototypes
// for SDL2 user interface functions (process inputs, draw outputs).

// main loop states
enum main_loop_e {quit, running, open, door, wall, invisible_wall, battle};

//GLOBAL variables
extern SDL_Event event;        // current keyboard/mouse event
extern enum main_loop_e status;  // quit, running, active key mode (open, etc.)

// Statistics
extern int num_times_rendered;   // statistics of how many times screen was rendered.
extern int num_events_observed;  // statistics
extern int num_events_user;      // statistics
extern int num_events_window;    // statistics
extern int num_events_keyboard;  // statistics
extern int num_events_mouse;     // statistics
extern int num_events_player_step; // statistics
extern int num_events_animation; // statistics
extern int num_events_button;    // statistics
extern int num_battle_rounds;    // statistics

//Function Prototypes

// called by main
extern void main_user_interface_loop(void); // main input fuction
  extern int handle_user_event(SDL_Event event);
  extern int handle_keyboard_event(SDL_Event event);
  extern int handle_soft_ui_button_event(int button_num);
  extern int handle_mouse_event(SDL_Event event);
  extern int handle_window_event(SDL_Event event);

