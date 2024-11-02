// battle.c
#include "all_headers.h"

// GLOBAL variables
SDL_Texture *circle_black;
SDL_Texture *circle_white;
SDL_Texture *circle_green;
SDL_Texture *circle_red;
SDL_Texture *ring_blue;
SDL_Texture *knight_silhouette;
SDL_Texture *pacman_ghost;

float radius_black = 1.00;
float radius_white = 0.75;
float radius_green = 0.25;
float radius_red   = 0.05;
int max_swing_time = 500;

int battle_timer;

SDL_Point target_pc_center;
SDL_Point target_monster_center;
int target_scale;

struct battle_target_s monster_targets[10]; // monsters are the targets
struct battle_target_s pc_targets[10];      // pc is the target
int num_targets;
enum battle_phase_e battle_phase;


// Called at beginning of game to load textures.
void init_battle_arena(void) {
    circle_black = load_symbol_texture ("PNG/circle-white.png",color_from_rgba(COLOR_BLACK),2);
    circle_white = load_symbol_texture ("PNG/circle-white.png",color_from_rgba(COLOR_WHITE),2);
    circle_green = load_symbol_texture ("PNG/circle-white.png",color_from_rgba(COLOR_GREEN),2);
    circle_red = load_symbol_texture ("PNG/circle-white.png",color_from_rgba(COLOR_RED),2);
    ring_blue = load_symbol_texture ("PNG/ring-white.png",color_from_rgba(COLOR_BLUE),2);
    knight_silhouette = load_symbol_texture ("PNG/knight-silhouette.png",color_from_rgba(COLOR_BLACK),1);
    pacman_ghost = load_symbol_texture ("PNG/pacman-ghost-black.png",color_from_rgba(COLOR_BLACK),1);
};

// get a floating point random number between min and max
float frand (float min, float max) {
    float result;
    result = (float)rand()/RAND_MAX * (max-min) + min;
    return result;
}

// called at beginning of each battle to setup monsters and start battle sequencer.
void setup_new_battle(int num_battle_monsters) {
    int i;
    num_targets = num_battle_monsters;
    battle_timer = 0;
    for (i=0;i<num_monsters;i++) {
        // monster is the target
        monster_targets[i].locked = 0;
        monster_targets[i].x = frand(-1,1);
        monster_targets[i].y = frand(-1,1);
        monster_targets[i].speed = .05 +frand(0,.03);
        monster_targets[i].angle = frand(0,360);
        monster_targets[i].mnum = i;
        monster_targets[i].swingtime = -1;

        // pc is the target
        pc_targets[i].locked = 0;
        pc_targets[i].x = frand(-1,1);
        pc_targets[i].y = frand(-1,1);
        pc_targets[i].speed = .05 +frand(0,.03);
        pc_targets[i].angle = frand(0,360);
        pc_targets[i].mnum = i;
        pc_targets[i].swingtime = rand()%max_swing_time +1;
    }
    battle_phase = target;
    dirty_display = 65;
}
// Draw battlemap every screen refresh during dirty_display_handler
void draw_battlemap(void) {
    int i;
    // pc is the target, draw on right
    target_pc_center.x = screen_width*3/4;
    target_pc_center.y = screen_height/2;
    target_scale = fmin(screen_height/2,screen_width/4);
    draw_image(circle_black,
                target_pc_center.x,target_pc_center.y,
                2*target_scale*radius_black,2*target_scale*radius_black,0,0);
    draw_image(circle_white,
                target_pc_center.x,target_pc_center.y,
                2*target_scale*radius_white,2*target_scale*radius_white,0,0);
    draw_image(circle_green,
                target_pc_center.x,target_pc_center.y,
                2*target_scale*radius_green,2*target_scale*radius_green,0,0);
    draw_image(knight_silhouette,
                target_pc_center.x,target_pc_center.y,
                2*target_scale*radius_green,2*target_scale*radius_green,0,0);

    for (i=0;i<num_targets;i++) {
        draw_image(circle_red,
                target_pc_center.x + target_scale*pc_targets[i].x,
                target_pc_center.y + target_scale*pc_targets[i].y,
                2*target_scale*radius_red,
                2*target_scale*radius_red,0,0);
    }

    // monster is the target, draw on left
    target_monster_center.x = screen_width/4;
    target_monster_center.y = screen_height/2;
    target_scale = fmin(screen_height/2,screen_width/4);
    draw_image(circle_black,
                target_monster_center.x,target_monster_center.y,
                2*target_scale*radius_black,2*target_scale*radius_black,0,0);
    draw_image(circle_white,
                target_monster_center.x,target_monster_center.y,
                2*target_scale*radius_white,2*target_scale*radius_white,0,0);
    draw_image(circle_green,
                target_monster_center.x,target_monster_center.y,
                2*target_scale*radius_green,2*target_scale*radius_green,0,0);
    draw_image(pacman_ghost,
                target_monster_center.x,target_monster_center.y,
                2*target_scale*radius_green,2*target_scale*radius_green,0,0);

    for (i=0;i<num_targets;i++) {
        draw_image(circle_red,
                target_monster_center.x + target_scale*monster_targets[i].x,
                target_monster_center.y + target_scale*monster_targets[i].y,
                2*target_scale*radius_red,
                2*target_scale*radius_red,0,0);
    }
}

// This routine gets called by the interrupt handler every tick during a battle
void move_targets(void) {
    int i;
    int still_running =0;
    float distance;
    float angle;
    float dx, dy;
    SDL_FPoint center;
    center.x=0; center.y=0;
    battle_timer += 1;
    if (battle_phase==target) {
        for (i=0;i<num_targets;i++) {
            if (monster_targets[i].locked) continue;
            still_running++;
            dx = monster_targets[i].x - center.x; 
            dy = monster_targets[i].y - center.y; 
            distance = sqrt(pow(dx,2) + pow(dy,2));
            angle = atan2(dy, dx)*57.2;
            if (distance > radius_black) {
                monster_targets[i].angle=-angle-90-30+rand()%60;
            }
            monster_targets[i].x+= monster_targets[i].speed * sin(monster_targets[i].angle/57.2);
            monster_targets[i].y+= monster_targets[i].speed * cos(monster_targets[i].angle/57.2);
        }
        for (i=0;i<num_targets;i++) {
            if (pc_targets[i].locked) continue;
            still_running++;
            dx = pc_targets[i].x - center.x; 
            dy = pc_targets[i].y - center.y; 
            distance = sqrt(pow(dx,2) + pow(dy,2));
            angle = atan2(dy, dx)*57.2;
            if (distance > radius_black) {
                pc_targets[i].angle=-angle-90-30+rand()%60;
            }
            pc_targets[i].x+= pc_targets[i].speed * sin(pc_targets[i].angle/57.2);
            pc_targets[i].y+= pc_targets[i].speed * cos(pc_targets[i].angle/57.2);
            if (pc_targets[i].swingtime <= battle_timer) {
                pc_targets[i].locked = 1;
            }
        }
        if (still_running == 0) {
            battle_phase = score;
        } else {
            dirty_display = 66;
        }
    }
}

// called in response to player trigger during a targeting phase
void battle_swing(void) {
    int i;
    float distance;
    float dx, dy;
    char buf[50];
    SDL_FPoint center;
    center.x=0; center.y=0;

    for (i=0;i<num_targets;i++) {
        monster_targets[i].swingtime = battle_timer;
        monster_targets[i].locked = 1;
        dx = monster_targets[i].x - center.x; 
        dy = monster_targets[i].y - center.y; 
        distance = sqrt(pow(dx,2) + pow(dy,2));
        if (distance <= radius_green) {
            // critical hit
            snprintf(buf, 50, "Critical Hit monster %d",i);
            message_create(buf,0);
            printf("%s\n",buf);
        } else if (distance <= radius_white) {
            // hit
            snprintf(buf, 50, "Hit monster %d",i);
            message_create(buf,0);
            printf("%s\n",buf);
        } else {
            // miss
            snprintf(buf, 50, "Miss monster %d",i);
            message_create(buf,0);
            printf("%s\n",buf);
        }
    }
    //battle_phase = score;
}
