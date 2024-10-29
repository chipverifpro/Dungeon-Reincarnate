// sdl_gui.c
#include "all_headers.h"

// main loop states
//enum main_loop_e {quit, running, open, door, wall, invisible_wall, battle};
int mode_inventory = 0;

//globals
SDL_Event event;          // current key/mouse/finger/misc event to process
enum main_loop_e status;  // quit, running, active key mode (open, etc.)

int num_times_rendered;   // statistics of how many times screen was rendered.
int num_events_observed;  // statistics
int num_events_user;      // statistics
int num_events_window;    // statistics
int num_events_keyboard;  // statistics
int num_events_mouse;     // statistics
int num_events_player_step; // statistics
int num_events_animation; // statistics
int num_events_button;    // statistics
int num_battle_rounds;    // statistics

// parameter and history for multitouch gestures
SDL_Event initial_two_touch;     // starting point for two_touch (event structure)
float initial_sw_scale;          // scale before two_touch
float two_touch_dist_threshold;  // 
float two_finger_drag_dx;        // distance of current two_touch drag (float)
float two_finger_drag_dy;        //
float two_finger_pinch_dist;     // distance of current two_touch pinch (float)

//////////////////////////////////////////////////////////
// Main loop functions                                  //
//////////////////////////////////////////////////////////

int handle_user_event(SDL_Event event) {
    int mnum;
    num_events_user++;
    if (status == battle) {
        move_targets();
    }
    if (pl.route_following) {
        num_events_player_step ++;
        pl.route_following = player_follow_route();
        dirty_display = 1;
        //return(1);
    }
    for (mnum =0; mnum<num_monsters; mnum++) {
        if (monsters[mnum].map_number == map.map_number) {
            monster_random_walk(mnum);
        }
    }
    return(1);
}
        
int handle_keyboard_event(SDL_Event event) {
    int inum;
    int tnum;
    char buf[200];
    //int button_num;
    num_events_keyboard++;
    switch (event.type) {
        case SDL_KEYDOWN:
            printf("Key pressed: %s (player @ %f,%f)\n", SDL_GetKeyName(event.key.keysym.sym),pl.x,pl.y);
            switch (event.key.keysym.sym) {
                case SDLK_o:     status=open; break;
                case SDLK_d:     status=door; break;
                case SDLK_w:     status=wall; break;
                case SDLK_i:     status=invisible_wall; break;
                case SDLK_b:     status=battle;
                                 setup_new_battle(rand()%5+1);
                                 break;
                case SDLK_q:     status=quit; break;
                case SDLK_s:     write_map_csv();
                                 write_objects_csv("Maps/objects_out.csv");
                                 break;
                case SDLK_EQUALS:sw_scale += 1.5;
                                 SDL_DestroyTexture(texture);
                                 texture = NULL;
                                 dirty_display = 6;
                                 break;
                case SDLK_MINUS: sw_scale -= 1.5;
                                 SDL_DestroyTexture(texture);
                                 texture = NULL;
                                 dirty_display = 6;
                                 break;
                case SDLK_g:    inum = player_get_item(pl.x,pl.y);
                                if (inum == -1) {
                                    printf("Nothing to pick up here\n");
                                } else {
                                    dirty_display = 1;
                                    snprintf(buf,200,"You picked up %s",objects[inum].description);
                                    printf("%s\n",buf);
                                    message_create(buf,0);
                                }
                                break;
                case SDLK_p:    inum = player_drop_item(pl.x,pl.y);
                                if (inum == -1) {
                                    printf("Player has nothing to put down\n");
                                } else {
                                    dirty_display = 1;
                                    printf("player put down up %s\n",
                                        objects[inum].description);
                                }
                                break;
                case SDLK_v:    if (pl.x < 0 || pl.y < 0) break; // cannot grow map in negative direction
                                if (ifloor(pl.x)>=map.x_size) map.x_size=ifloor(pl.x)+1; // grow map
                                if (ifloor(pl.y)>=map.y_size) map.y_size=ifloor(pl.y)+1; // grow map
                                map.valid[ifloor(pl.x)][ifloor(pl.y)] ^= 0x1;
                                dirty_display = 1;
                                break;
                case SDLK_t:    tnum = map.walls[ifloor(pl.x)][ifloor(pl.y)] >> 12 & 0x0F;
                                tnum ++;
                                if (map.terrains[tnum].NameT==NULL || strlen(map.terrains[tnum].NameT) <2) {
                                    tnum=0;
                                }
                                //printf("Terrain = %d\n",tnum);
                                map.walls[ifloor(pl.x)][ifloor(pl.y)] &= 0x0FFF;
                                map.walls[ifloor(pl.x)][ifloor(pl.y)] += tnum<<12;
                                dirty_display = 1;
                                break;
                case SDLK_SPACE: pl.route_following = 1;
                                pl.route_following = player_follow_route();
                                dirty_display = 1;
                                break;
                //case SDLK_i:    print_player_inventory();
                //                break;
                case SDLK_LSHIFT:
                case SDLK_RSHIFT: mode_inventory = 1;
                                break;
            }
            break;
        case SDL_KEYUP:
            switch (event.key.keysym.sym) {
                case SDLK_LSHIFT: 
                case SDLK_RSHIFT:
                    printf("Key up: %s (player @ %f,%f)\n", SDL_GetKeyName(event.key.keysym.sym),pl.x,pl.y);
                    mode_inventory = 0; break;
            }
    }
    if (status == running && event.type==SDL_KEYDOWN) {
        switch (event.key.keysym.sym) {
            case SDLK_UP:    num_events_player_step++; pl.route_following = 0; player_move(0,pl.step_size); dirty_display = 1; break;
            case SDLK_RIGHT: num_events_player_step++; pl.route_following = 0; player_move(1,pl.step_size); dirty_display = 1; break;
            case SDLK_DOWN:  num_events_player_step++; pl.route_following = 0; player_move(2,pl.step_size); dirty_display = 1; break;
            case SDLK_LEFT:  num_events_player_step++; pl.route_following = 0; player_move(3,pl.step_size); dirty_display = 1; break;
        }
    } else if (status == open) {
        switch (event.key.keysym.sym) {
            case SDLK_UP:    
                pl.dir = 0; player_open(pl.dir);
                dirty_display = 1;
                status=running; break;
            case SDLK_RIGHT:
                pl.dir = 1; player_open(pl.dir);
                dirty_display = 1;
                status=running; break;
            case SDLK_DOWN:
                pl.dir = 2; player_open(pl.dir);
                dirty_display = 1;
                status=running; break;
            case SDLK_LEFT:
                pl.dir = 3; player_open(pl.dir);
                dirty_display = 1;
                status=running; break;
            case SDLK_PERIOD:
                player_open_chest(pl.x, pl.y);
                dirty_display = 1;
                status=running; break;
        }
    } else if (status == wall) { // map editor feature
        switch (event.key.keysym.sym) {
            case SDLK_UP:    toggle_wall_xy(pl.x,pl.y,0,0); status=running; dirty_display = 1; break;
            case SDLK_RIGHT: toggle_wall_xy(pl.x,pl.y,1,0); status=running; dirty_display = 1; break;
            case SDLK_DOWN:  toggle_wall_xy(pl.x,pl.y,2,0); status=running; dirty_display = 1; break;
            case SDLK_LEFT:  toggle_wall_xy(pl.x,pl.y,3,0); status=running; dirty_display = 1; break;
            //case SDLK_UP:    map.walls[ifloor(pl.x)][ifloor(pl.y)] ^= (1<<0); status=running; dirty_display = 1; break;
            //case SDLK_RIGHT: map.walls[ifloor(pl.x)][ifloor(pl.y)] ^= (1<<1); status=running; dirty_display = 1; break;
            //case SDLK_DOWN:  map.walls[ifloor(pl.x)][ifloor(pl.y)] ^= (1<<2); status=running; dirty_display = 1; break;
            //case SDLK_LEFT:  map.walls[ifloor(pl.x)][ifloor(pl.y)] ^= (1<<3); status=running; dirty_display = 1; break;
        }
    } else if (status == door) { // map editor feature
        switch (event.key.keysym.sym) {
            case SDLK_UP:    toggle_wall_xy(pl.x,pl.y,0,4); status=running; dirty_display = 1; break;
            case SDLK_RIGHT: toggle_wall_xy(pl.x,pl.y,1,4); status=running; dirty_display = 1; break;
            case SDLK_DOWN:  toggle_wall_xy(pl.x,pl.y,2,4); status=running; dirty_display = 1; break;
            case SDLK_LEFT:  toggle_wall_xy(pl.x,pl.y,3,4); status=running; dirty_display = 1; break;
            //case SDLK_UP:    map.walls[ifloor(pl.x)][ifloor(pl.y)] ^= (1<<4); status=running; dirty_display = 1; break;
            //case SDLK_RIGHT: map.walls[ifloor(pl.x)][ifloor(pl.y)] ^= (1<<5); status=running; dirty_display = 1; break;
            //case SDLK_DOWN:  map.walls[ifloor(pl.x)][ifloor(pl.y)] ^= (1<<6); status=running; dirty_display = 1; break;
            //case SDLK_LEFT:  map.walls[ifloor(pl.x)][ifloor(pl.y)] ^= (1<<7); status=running; dirty_display = 1; break;
        }
    } else if (status == invisible_wall) { // map editor feature
        switch (event.key.keysym.sym) {
            case SDLK_UP:    toggle_wall_xy(pl.x,pl.y,0,8); status=running; dirty_display = 1; break;
            case SDLK_RIGHT: toggle_wall_xy(pl.x,pl.y,1,8); status=running; dirty_display = 1; break;
            case SDLK_DOWN:  toggle_wall_xy(pl.x,pl.y,2,8); status=running; dirty_display = 1; break;
            case SDLK_LEFT:  toggle_wall_xy(pl.x,pl.y,3,8); status=running; dirty_display = 1; break;
            //case SDLK_UP:    map.walls[ifloor(pl.x)][ifloor(pl.y)] ^= (1<<8); status=running; dirty_display = 1; break;
            //case SDLK_RIGHT: map.walls[ifloor(pl.x)][ifloor(pl.y)] ^= (1<<9); status=running; dirty_display = 1; break;
            //case SDLK_DOWN:  map.walls[ifloor(pl.x)][ifloor(pl.y)] ^= (1<<10); status=running; dirty_display = 1; break;
            //case SDLK_LEFT:  map.walls[ifloor(pl.x)][ifloor(pl.y)] ^= (1<<11); status=running; dirty_display = 1; break;
        }
    } else if (status == battle && event.type==SDL_KEYDOWN) { // in battle mode
        switch (event.key.keysym.sym) {
            case SDLK_SPACE:
                battle_swing();
                dirty_display = 68;
                break;
            case SDLK_x:
                status = running;
                dirty_display = 67;
                break;
        }
    }
    return(1);
}

int handle_soft_ui_button_event(int button_num) {
    int uid;
    int obj;
    char buf[200];
    int dest_map;
    float dest_x, dest_y;
    int dest_dir;
    
    // SOFT UI BUTTONS
    num_events_button++;
    printf("%s Button (%d) clicked on\n", button_list[button_num].text, button_num);
    uid = button_list[button_num].object_uid;
    obj = get_object_by_uid(uid);
    //printf("Associated object is %d:%c:%c:%s\n",obj,objects[obj].owner,objects[obj].common_type,objects[obj].description);
    switch(objects[obj].owner) {
        case 'M':   // item is on map
            switch(objects[obj].common_type) {
                case '$': // gold
                case 'A': // armor
                case 'W': // weapon
                case 'K': // key
                    player_get_item_num(obj);
                    snprintf(buf,200,"You picked up %s",objects[obj].description);
                    printf("%s\n",buf);
                    //message_create(buf,uid);
                    break;
                case 'S': // stairs
                    //message_create(buf,uid,NULL);
                    printf("Destination would be %s\n",objects[obj].custom_data);
                    dest_map = 0;
                    sscanf(objects[obj].custom_data,"Map%d@%f,%f,%d",
                        &dest_map, &dest_x, &dest_y, &dest_dir);
                    printf("Destination parsed: dest_map=%d, dest_xy=%2.2f,%2.2f, dest_dir=%d\n",
                        dest_map, dest_x, dest_y, dest_dir);
                    if (dest_map >=1 && dest_map < 10) {
                        // back-up the existing map before loading the new one.
                        memcpy(&maps[current_map_num],&map,sizeof(struct map_s));
                        valid_maps[current_map_num]=1;
                        if (valid_maps[dest_map]==1) {
                            memcpy(&map,&maps[dest_map],sizeof(struct map_s));
                            current_map_num = dest_map;
                        } else {
                            read_map_csv(dest_map);
                        }
                        player_move_to(dest_x, dest_y, dest_dir);
                        dirty_display = 7;
                        snprintf(buf,200,"You climbed the %s to %s",objects[obj].description, map.map_name);
                        printf("%s\n",buf);
                        message_create(buf,uid);
                    } else {
                        snprintf(buf,200,"Climb the stairs: The %s are closed for construction.",objects[obj].description);
                        printf("%s\n",buf);
                        message_create(buf,uid);
                    }
                    break;
                case 'N': // NPC
                    snprintf(buf,200,"%s responds: %s",objects[obj].description,objects[obj].custom_data);
                    printf("%s\n",buf);
                    message_create(buf,uid);
                    break;
                case 'F': // Fountain
                    snprintf(buf,200,"A drink from the %s tastes like %s\n",objects[obj].description,objects[obj].custom_data);
                    printf("%s",buf);
                    message_create(buf,uid);
                    break;
                case 'C': // Chest
                    player_open_chest(pl.x,pl.y);
                    dirty_display=4;
                    break;
                default:
                    break;
            }
            //printf("end of on-map item button events\n");
            break;
        case 'P':   // player owns item
            //TODO:player_drop_item
            //TODO: Use Key
            break;
        default:
            break;
    }
    return(1);
}

int handle_mouse_event(SDL_Event event) {
    int button_num;
    switch (event.type) {
        case SDL_MOUSEBUTTONDOWN:
            num_events_mouse++;
            printf("Mouse button pressed: %d @ %d,%d\n", event.button.button, event.button.x, event.button.y);

            // Mouse Clicked on SOFT UI BUTTON
            if ((button_num=which_button(event.button.x, event.button.y)) != -1) {
                handle_soft_ui_button_event(button_num);
                break;
            }

            // route player to mouse click location
            struct F_Point_s target = convert_scr_to_map(event.button.x, event.button.y);
            pl.target_x = event.button.x;
            pl.target_y = event.button.y;
            int success = plan_route(pl.target_map, target.x,target.y,pl.option_open_doors,1);
            if (success) {
                printf("plan_route() returned SUCCESS\n");
                pl.route_following = 1;
            } else {
                printf("plan_route() returned FAILURE\n");
                pl.route_following = 0;
            }
            printf("Mouse button pressed at %f,%f\n",target.x,target.y);
            break;
        case SDL_FINGERUP:
            num_events_mouse++;
            initial_two_touch.mgesture.touchId=0;
            //printf("Two-Touch gesture ends: Finger up.\n");
            break;
        case SDL_FINGERDOWN:
            num_events_mouse++;
            initial_two_touch.mgesture.touchId=0;
            //printf("Two-Touch gesture ends: Finger down.\n");
            break;
        case SDL_MULTIGESTURE:
            num_events_mouse++;
            if (event.mgesture.numFingers == 2) {
                if (initial_two_touch.mgesture.touchId == 0) {
                    initial_two_touch = event;  // save for later comparisons
                    initial_sw_scale = sw_scale;
                    //printf("Two-Touch gesture begins:    id=%llu,x=%f,y=%f,dist=%f,theta=%f\n",
                    //        event.mgesture.touchId, event.mgesture.x, event.mgesture.y,
                    //        event.mgesture.dDist, event.mgesture.dTheta);
                };
                if (event.mgesture.dDist > two_touch_dist_threshold) {
                    //printf("Two-Touch gesture continues: id=%llu,x=%f,y=%f,dist=%f,theta=%f\n",
                            //event.mgesture.touchId, event.mgesture.x, event.mgesture.y,
                            //event.mgesture.dDist, event.mgesture.dTheta);
                    two_finger_drag_dx = event.mgesture.x - initial_two_touch.mgesture.x;
                    two_finger_drag_dy = event.mgesture.y - initial_two_touch.mgesture.y;
                    sw_scale = initial_sw_scale + (initial_sw_scale * two_finger_drag_dy / 1.0);
                    two_finger_pinch_dist = event.mgesture.dDist - initial_two_touch.mgesture.dDist;
                    //sw_scale = sw_scale + (sw_scale * two_finger_pinch_dist / 1.0);
                    //sw_scale = sw_scale + (sw_scale * event.mgesture.dDist / 1.0);
                    //two_finger_dist = 
                    dirty_display = 1;
                    //printf("Two-Touch gesture continues: \tdelta=%f,%f, \tpinch=%f\n",
                    //        two_finger_drag_dx, two_finger_drag_dy, two_finger_pinch_dist);

                }
            } else {
                // two finger gesture is done
                initial_two_touch.mgesture.touchId = 0;
                printf("Two-Touch gesture complete:  id=%llu,x=%f,y=%f,dist=%f,theta=%f\n",
                        event.mgesture.touchId, event.mgesture.x, event.mgesture.y,
                        event.mgesture.dDist, event.mgesture.dTheta);
            }
            break;
    }
    return(1);
}

int handle_window_event(SDL_Event event) {
    num_events_window++;
    if (event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
        screen_width  = event.window.data1;
        screen_height = event.window.data2;
        printf("Window size changed to %dx%d\n", screen_width, screen_height);
        dirty_display = 1;
        return(1);
    }
    return (0);
}

void main_user_interface_loop(void) {
    status = running;

    while (status != quit) {
        // Event loop
        dirty_display_handler();

        // Process the events
        //while (SDL_PollEvent(&event)) {
        SDL_WaitEvent(&event);
        if (1==1) {
            num_events_observed++;
            switch (event.type) {
                case SDL_QUIT:
                    status=quit; // Exit the loop due to window close
                    break;

                case SDL_USEREVENT:
                    handle_user_event(event);
                    break;

                case SDL_KEYDOWN:
                case SDL_KEYUP:
                    handle_keyboard_event(event);
                    break;

                case SDL_MOUSEBUTTONDOWN:
                case SDL_FINGERUP:
                case SDL_FINGERDOWN:
                case SDL_MULTIGESTURE:
                    handle_mouse_event(event);
                    break;

                case SDL_WINDOWEVENT:
                    handle_window_event(event);
                    break;
            }
        }
        // TODO: Do Object BumpBacks
        do_object_bumpbacks();
    }
    // end of game, print some debug stuff...
    //print_player_inventory();
    print_statistics();
}
