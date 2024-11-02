// messages.h

// GLOBAL Variables

extern struct button_s *messages; // list of messages
extern int *life;

//extern int num_messages;                // number of messages
extern int messages_head;                // messages is a circular buffer
extern int message_font_size;            // global font size for messages
extern int message_border_size;          // pixels added to text each side
extern int message_spacing;              // space between two messages
extern int messages_from_edge_spacing;   // Don't draw messages right at edge
// Function Prototypes
//extern void start_ttf(void);    // library and font setup
//extern void quit_ttf(void);     // library and font destroy

extern void init_messages(void);    // malloc some memory

extern SDL_Texture *message_create_texture(int message_num);

// called from user_input::handle_soft_ui_button_event (NPC, Fountain)
extern int message_create(char *text, int uid); // creation

// not yet used
extern int message_destroy(int message_num);      // valid = 0;

// called from sdl_draw::dirty_display_handler
extern int arrange_and_draw_messages(void);      // arrange and draw all message
  extern int draw_one_message(int message_num);     // render a single message

// not yet used
extern int which_message(int scr_x, int scr_y);  // converts screen coordinates to button number
