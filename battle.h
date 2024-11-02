// battle.h
struct battle_target_s {
  float x;
  float y;
  float speed;
  float angle;
  int locked;
  int swingtime;
  int mnum;
};

// GLOBAL variables
extern SDL_Texture *circle_black;
extern SDL_Texture *circle_white;
extern SDL_Texture *circle_green;
extern SDL_Texture *circle_red;
extern SDL_Texture *ring_blue;

extern int battle_timer;

extern struct battle_target_s monster_targets[10]; // monsters are the targets
extern struct battle_target_s pc_targets[10];      // pc is the target
extern int num_targets;

enum battle_phase_e {countdown, target, score, complete};
extern enum battle_phase_e battle_phase;

// Function prototypes

// Called at beginning of game to load textures.
extern void init_battle_arena(void);

// get a floating point random number between min and max
extern float frand (float min, float max);

// called at beginning of each battle to setup monsters and start battle sequencer.
extern void setup_new_battle(int num_battle_monsters);

// Draw battlemap every screen refresh during SDL_DRAW::dirty_display_handler
extern void draw_battlemap(void);

// This routine gets called by the interrupt handler every tick during a battle
extern void move_targets(void);

// called in response to player trigger during a targeting phase
extern void battle_swing(void);
