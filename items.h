// items.h
// This file contains global structs and functions relating to items and other objects.

//enum trap_type_e {
//    none, pit, spikes, poison_needle, gas, arrows, falling_block
//}

struct chest_s {
    int uid;
    int locked;
    int trapped;
    int closed;
    int empty;
    int is_mimic;
    int key_needed_uid;
    char *trap_text;
    // various action buttons can apply.  These are their button numbers.
    int button_open;
    int button_close;
    int button_unlock;
    int button_disarm;
    int button_examine;
    int button_force;
};

// Function prototypes

// called by user_input::
extern int player_get_item(float x, float y);        // player picks up first item found within reach radius.
extern int player_get_item_num(int obj);          // Grab an item that is in reach
extern int player_drop_item(float x, float y);
extern int player_drop_item_num(int obj_num, float x, float y);
extern int player_open_chest(float x, float y);
  extern int transfer_items (int old_owner_uid, char new_owner, int new_owner_num);

// called by user_input::main_loop
int do_object_bumpbacks(void);

//called by user_input::
extern int get_object_by_uid(int uid);

//called by user_input::
extern void print_player_inventory(void);
