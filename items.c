// items.c
// This file contains functions relating to items and other objects.
#include "all_headers.h"

// picks up first available object within reach radius.
// TODO: Find closest object first.
int player_get_item(float x, float y) {
    float dist;
    float reach=0.5;
    for (int i=0; i<num_objects;i++) {
        dist = get_distance (x,y, objects[i].x,objects[i].y);
        if (dist <= reach) {
            if (       (objects[i].valid == 1)
                    && (objects[i].permanent==0)
                    && (objects[i].held == 0)
                    && (objects[i].owner_num == map.map_number)
                    && (objects[i].owner=='M')) {
                objects[i].held = 1;
                objects[i].x = -1;
                objects[i].y = -1;
                objects[i].owner = 'P';
                objects[i].owner_num = 1;
                //pl.num_objects ++;
                return (i);
            }
        }
    }
    return(-1);
}

extern int player_get_item_num(int obj)  {        // Grab an item by number
    objects[obj].held = 1;
    objects[obj].x = -1;
    objects[obj].y = -1;
    objects[obj].owner = 'P';
    objects[obj].owner_num = 1;
    //pl.num_objects ++;
    dirty_display = 9;
    return (obj);
};

// drops first available object in player's inventory.
int player_drop_item(float x, float y) {
    for (int i=0; i<num_objects;i++) {
        if (           (objects[i].owner=='P')
                    && (objects[i].valid == 1) 
                    && (objects[i].permanent==0)
                    && (objects[i].held == 1)
                    && (objects[i].owner_num == 1))
            {
                objects[i].held = 0;
                objects[i].owner_num = map.map_number;
                objects[i].owner = 'M';
                objects[i].x = x;
                objects[i].y = y;
                //pl.num_objects--;
                printf("Player dropped %s\n", objects[i].description);
                return (i);
            }
    }
    return(-1);
}

// drops first available object in player's inventory.
int player_drop_item_num(int obj_num, float x, float y) {
    if ( (objects[obj_num].owner=='P')
                && (objects[obj_num].valid == 1) 
                && (objects[obj_num].permanent==0)
                && (objects[obj_num].held == 1)
                && (objects[obj_num].owner_num == 1)) {
        objects[obj_num].held=0;
        objects[obj_num].owner_num = map.map_number;
        objects[obj_num].owner='M';
        objects[obj_num].x = x;
        objects[obj_num].y = y;
        //pl.num_objects--;
        printf("Player dropped %s\n",objects[obj_num].description);
        return(obj_num);
    }
    return(-1);
}

// this is useful for transferring all the objects belonging to one UID (Chest or monster) to a destination (Map or player, etc)
int transfer_items (int old_owner_uid, char new_owner, int new_owner_num) {
    int k;
    int num_removed_from_chest = 0;
    int old_owner_object = get_object_by_uid(old_owner_uid);
    for (k=0; k<num_objects;k++) {
        if (       (objects[k].valid == 1) 
                && (objects[k].owner_num == old_owner_uid)
                && (objects[k].owner=='U')) {
            // move objects from old owner to new.
            objects[k].x = objects[old_owner_object].x;
            objects[k].y = objects[old_owner_object].y;
            objects[k].owner = new_owner;
            objects[k].owner_num = new_owner_num;
            printf("Found %s\n",objects[k].description);
            num_removed_from_chest++;
        }
    }
    return (num_removed_from_chest);
}

// Open a chest and place all contained objects on the map at it's location.
int player_open_chest(float x, float y) {
    int chest_uid = -1;
    int new_chest_uid;
    int new_chest_object;
    int key_uid, key_object;
    int num_removed_from_chest = 0;
    float dist;
    float reach=0.5;
    char buf[200];
    printf("Player open chest at %f,%f\n",x,y);
    for (int i=0; i<num_objects;i++) {
        dist = get_distance (x,y, objects[i].x,objects[i].y); 
        if (dist <= reach) {
            if (       (objects[i].valid == 1) 
                    && (objects[i].common_type=='C') // container/chest
                    && (objects[i].owner_num == map.map_number)
                    && (objects[i].owner=='M')) {
                chest_uid = objects[i].uid;
                key_uid = atoi(objects[i].custom_data);
                if (key_uid >0) {
                    key_object = get_object_by_uid(key_uid);  // Does player 1 hold th key?
                    if (objects[key_object].owner != 'P' || objects[key_object].owner_num!=1) {
                        snprintf(buf,200,"You need the %s to %s %s",objects[key_object].description,
                                                            objects[i].button_text,
                                                            objects[i].description);
                        printf("%s\n",buf);
                        message_create(buf,objects[i].uid);
                        return (0);
                    } else {
                        snprintf(buf,200,"You used the %s to %s %s",objects[key_object].description,
                                                            objects[i].button_text,
                                                            objects[i].description);
                        printf("%s\n",buf);
                        message_create(buf,objects[i].uid);
                    }
                } else {
                    //printf("No key needed !\n");
                }
                switch(objects[i].button_text[0]) {
                    case 'S': //study map
                        new_chest_uid = chest_uid+1;
                        snprintf(buf,200,"%s",objects[i].custom_data);
                        printf("%s\n",buf);
                        message_create(buf,chest_uid);
                        break;
                    case 'U': //unlock
                        new_chest_uid = chest_uid+1;
                        break;
                    case 'D': //dig up
                        new_chest_uid = chest_uid+1;
                        break;
                    case 'O': //open
                        num_removed_from_chest = transfer_items (chest_uid, 'M', map.map_number);
                        new_chest_uid = chest_uid+1;
                        break;
                    case 'C': //close
                        //num_removed_from_chest = transfer_items (chest_uid, 'M', map.map_number);
                        new_chest_uid = chest_uid-1;
                        break;
                    default: // unknown chest type
                        new_chest_uid = chest_uid-1;
                        break;
                }

                // NEW METHOD: Replace the chest with an open one
                objects[i].valid = 0;   // disable locked chest
                printf("new_chest_uid = %d\n",new_chest_uid);
                new_chest_object = get_object_by_uid(new_chest_uid);
                printf("new_chest_object=%d\n",new_chest_object);
                objects[new_chest_object].valid = 1;
                
                if (num_removed_from_chest > 0) {
                    snprintf(buf,200,"%s opened and %d objects found",objects[i].description,num_removed_from_chest);
                    printf("%s\n",buf);
                    message_create(buf,new_chest_uid);
                };
                return (num_removed_from_chest);
            }
        }
    }
    return(-1);
}

int get_object_by_uid(int uid) {
    for (int i=0;i<num_objects;i++) {
        if (objects[i].uid == uid) {
            return (i);
        };
    };
    return(-1);
}

void print_player_inventory(void) {
    printf("Player Inventory:\n");
    for (int i=0;i<num_objects;i++) {
        if (objects[i].owner=='P' && objects[i].owner_num==1) {
            printf(" %s\n", objects[i].description);
        }
    }
}

int do_object_bumpbacks(void) {
    char buf[100];
    float cur_dx, cur_dy, new_dx, new_dy, pct_dist;
    float dist, target_dist;
    int orig_dir;
    for (int i=0;i<num_objects;i++) {
        if (objects[i].owner=='M' && objects[i].owner_num==map.map_number
                && objects[i].valid && objects[i].keepout >0) {
            orig_dir = pl.dir;
            dist = get_distance(pl.x, pl.y, objects[i].x, objects[i].y);
            if (dist < objects[i].keepout/2.0) {
                // Before doing bump-back, see if we need to stop following due to one or both reasons:
                //   EITHER the target is IN the object, or the object is IN the current player cell.
                //   Both can cause infinite loops in route following.
                target_dist = get_distance(pl.target_x, pl.target_y, objects[i].x, objects[i].y);
                if (target_dist < objects[i].keepout/2.0) {
                    // stop route following, target is in bump_back radius
                    pl.route_following = 0;
                    snprintf(buf,100,"Target is in object %s",objects[i].description);
                    printf("%s\n",buf);
                    //message_create(buf,objects[i].uid);
                }
                if ((ifloor(pl.target_x)==ifloor(objects[i].x)) && (ifloor(pl.target_y)==ifloor(objects[i].y))) {
                    // stop route following, target is in same cell as object
                    pl.route_following = 0;
                    snprintf(buf,100,"Target is in object %s's tile",objects[i].description);
                    printf("%s\n",buf);
                    //message_create(buf,objects[i].uid);
                } 
                // bump player back
                cur_dx = pl.x - objects[i].x;
                cur_dy = pl.y - objects[i].y;
                pct_dist = dist / (objects[i].keepout/2.0); // how much bounce needed
                new_dx = cur_dx * pct_dist; 
                new_dy = cur_dy * pct_dist;
                if (new_dx==0.00) new_dx+=(rand()%10)/100.0;  // in case of direct hit, steer to one side.
                if (new_dy==0.00) new_dy+=(rand()%10)/100.0;
                // bump the player, but don't allow pushing through walls
                if (new_dx >0) player_move(1,new_dx);
                if (new_dx <0) player_move(3,-new_dx);
                if (new_dy >0) player_move(2,new_dy);
                if (new_dy <0) player_move(0,-new_dy);
                snprintf(buf,100,"Bump into %s by %.0f%%",objects[i].description,(1.0-pct_dist)*100.0);
                printf("%s\n",buf);
                //message_create(buf,objects[i].uid);
                pl.dir = orig_dir;
                dirty_display = 10;
            }
        }
    }
    return (0);
}
