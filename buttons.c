// sdl_buttons.c
#include "all_headers.h"

/*
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>

#include "dungeon.h"
#include "items.h"
#include "sdl_time.h"
#include "sdl_draw.h"
#include "player.h"
#include "map_io.h"
#include "buttons.h"
*/

// Global Variables
TTF_Font *font;

struct button_s button_list[100]; // list of buttons
int num_buttons=0;               // number of buttons in list

// Local Globals
int button_font_size=20;         // global font size for buttons
int button_border_size = 2;      // pixels added to text each side
int button_spacing = 4;          // space between two buttons
int buttons_from_edge_spacing = 8; // Don't draw buttons right at edge

SDL_Color color_from_rgba(int r, int g, int b, int a) {
    SDL_Color dest;
    dest.r = r;  dest.g = g;  dest.b = b;  dest.a = a;
    return (dest);
}

void start_ttf(void) {
    if(TTF_Init()==-1) {
        printf("TTF_Init: %s\n", TTF_GetError());
        exit(2);
    }
    font=TTF_OpenFont("FONTS/SFNS.ttf", button_font_size);
    if(!font) {
        printf("TTF_OpenFont: %s\n", TTF_GetError());
        exit(2);
    }
}

void quit_ttf(void) {
    if (font != NULL) {
        TTF_CloseFont(font);
        font=NULL;
    };
    // destroy all button textures...
    for (int i=0;i<num_buttons;i++) {
        if (button_list[i].texture!=NULL) {
            SDL_DestroyTexture(button_list[i].texture);
            button_list[i].texture = NULL;
            button_list[i].valid = 0;
        }
    }
    TTF_Quit();
}

SDL_Texture *button_create_texture(int button_num) {
    SDL_Surface *message_surface;
    SDL_Texture *message_texture;

    // clear any previously created textures and re-create it.
    if (button_list[button_num].texture!=NULL) {
        SDL_DestroyTexture(button_list[button_num].texture);
        button_list[button_num].texture = NULL;
    }
    // determine width and height of text
    TTF_SizeText(font, button_list[button_num].text,
                 &button_list[button_num].rect.w,
                 &button_list[button_num].rect.h);
    button_list[button_num].rect.x = 0;  // button manager will override
    button_list[button_num].rect.y = 0;
    // render the text to a surface, then copy that to a texture
    message_surface = TTF_RenderText_Blended(font, button_list[button_num].text, button_list[button_num].fg_color);
    button_list[button_num].rect.h = message_surface->h + 2*button_border_size;
    button_list[button_num].rect.w = message_surface->w + 2*button_border_size;
    if (message_surface==NULL) {
        printf("TTF_RenderText_Blended FAILED: %s\n",SDL_GetError());
        exit(-1);
    }
    message_texture = SDL_CreateTextureFromSurface(renderer, message_surface);
    if (message_texture==NULL) {
        printf("TTF_CreateTextureFromSurface FAILED: %s\n",SDL_GetError());
        exit(-1);
    }
    SDL_FreeSurface(message_surface);

    return (message_texture);
}

int button_create(char *text, int uid) {
    SDL_Color message_color;
    SDL_Color background_color;
    SDL_Color border_color;

    message_color    = color_from_rgba(COLOR_WHITE);
    background_color = color_from_rgba(COLOR_BLACK_TRANSPARENT);
    border_color     = color_from_rgba(COLOR_YELLOW);

    button_list[num_buttons].text = str_alloc_copy(text);
    button_list[num_buttons].visible = 1;
    button_list[num_buttons].valid = 1;
    button_list[num_buttons].object_uid = uid;
    button_list[num_buttons].fg_color = message_color;
    button_list[num_buttons].bg_color = background_color;
    button_list[num_buttons].border_color = border_color;

    //
    button_list[num_buttons].texture = button_create_texture(num_buttons);

    //
    num_buttons++;
    return (num_buttons-1); // return the created button number
}

//int button_hide(int button_num) {
//    if ((button_num>=num_buttons) || (button_list[button_num].valid == 0)) {
//        return (-1);
//    }
//    printf("Making button_num %d hidden (%s)\n",button_num,button_list[button_num].text);
//    button_list[button_num].visible = 0;
//    return (button_num);
//}

//int button_show(int button_num) {
//    if ((button_num>=num_buttons) || (button_list[button_num].valid == 0)) {
//        return (-1);
//    }
//    printf("Making button_num %d visible (%s)\n",button_num,button_list[button_num].text);
//    button_list[button_num].visible = 1;
//    return (button_num);
//}

//int button_destroy(int button_num) {
//    if (button_num>=num_buttons) {
//        return(-1);
//    };
//    if (button_list[button_num].texture!=NULL) {
//        SDL_DestroyTexture(button_list[button_num].texture);
//        button_list[button_num].texture = NULL;
//    }
//    button_list[button_num].valid = 0;
//    return (0);
//}

int draw_one_button(int button_num) {    // render a single button
    SDL_Rect button_rect;
    if ((button_num>=num_buttons) || (button_list[button_num].valid == 0)) {
        printf("Requested button %d not valid\n",button_num);
        return (-1);
    }
    // Draw the Background
    button_rect = button_list[button_num].rect;
    SDL_SetRenderDrawColor(renderer, button_list[button_num].bg_color.r,
                                   button_list[button_num].bg_color.g,
                                   button_list[button_num].bg_color.b,
                                   button_list[button_num].bg_color.a);
    SDL_RenderFillRect(renderer, &button_rect);
    // Draw the Frame
    SDL_SetRenderDrawColor(renderer, button_list[button_num].border_color.r,
                                   button_list[button_num].border_color.g,
                                   button_list[button_num].border_color.b,
                                   button_list[button_num].border_color.a);
    SDL_RenderDrawRect(renderer, &button_rect);
    // Draw the Text Label
    button_rect.x += button_border_size; // shrink rect to text size
    button_rect.y += button_border_size;
    button_rect.h -= 2*button_border_size;
    button_rect.w -= 2*button_border_size;
    SDL_RenderCopy(renderer, button_list[button_num].texture, NULL, &button_rect);
    return (button_num);
}

// Upper left corner buttons version
int arrange_and_draw_UpperLeft_buttons(void) {     // arrange and draw all active buttons
    int number_drawn =0;   // statistics
    int x,y;    // upper left of the current button
    int i;

    x=buttons_from_edge_spacing;
    y=buttons_from_edge_spacing;

    for (i=0;i<num_buttons;i++) {
        if (button_list[i].valid && button_list[i].visible) {
            button_list[i].rect.x = x;
            button_list[i].rect.y = y;
            draw_one_button(i);
            y+=button_list[i].rect.h + 2*button_border_size + button_spacing;
            number_drawn++;
        }
    }

    return(number_drawn);
}

// Lower left corner buttons version
int arrange_and_draw_buttons(void) {
    int number_drawn =0;   // statistics
    int x,y;    // lower left of the current button
    int i;

    x=buttons_from_edge_spacing;
    y=screen_height - buttons_from_edge_spacing;

    for (i=0;i<num_buttons;i++) {
        if (button_list[i].valid && button_list[i].visible) {
            button_list[i].rect.x = x;
            button_list[i].rect.y = y - button_list[i].rect.h - 2*button_border_size;
            draw_one_button(i);
            y-=button_list[i].rect.h + 2*button_border_size + button_spacing;
            number_drawn++;
        }
    }

    return(number_drawn);
} 

// functions to look up button number...
int which_button(int scr_x, int scr_y) { // converts screen coordinates to visible button number
    int i;
    SDL_Point scr;
    scr.x = scr_x;  scr.y = scr_y;
    for (i=0;i<num_buttons;i++) {
        if ((button_list[i].valid == 0) || (button_list[i].visible == 0)) {
            continue;
        }
        if (SDL_PointInRect(&scr,&button_list[i].rect)) {
            return(i);
        }
    }   
    return(-1);
}

//int find_buttons_by_uid (int uid, int *matching_buttons) { // looks for object UID, returns button number list
//    int i;
//    int num_matching=0;
//    for (i=0;i<num_buttons;i++) {
//        if (button_list[i].valid == 0) {
//            continue;
//        }
//        if (button_list[i].object_uid == uid) {
//            matching_buttons[num_matching] = i;
//            num_matching++;
//        }
//    }
//    return(num_matching);
//}

int enable_disable_buttons_by_uid (int uid, int enable) { // enables or disables all buttons relating to an object UID
    int i;
    int num_matching=0;
    for (i=0;i<num_buttons;i++) {
        if (button_list[i].valid == 0) {
            button_list[i].visible = 0;
            continue;
        }
        if (button_list[i].object_uid == uid) {
            button_list[i].visible = enable;
            //printf("Button %d visible=%d :: %s\n",i,enable,button_list[i].text);
            dirty_display = 5;
            num_matching++;
        }
    }
    //printf("Found %d buttons matching object UID %d\n",num_matching, uid);
    return(num_matching);
}

void create_all_object_buttons(void) {
    int i;
    char buf[200];
    for (i=0;i<num_objects;i++){
        if (objects[i].uid != 0) {
            if (strlen(objects[i].button_text) != 0) {
                snprintf(buf,200,"%s %s",objects[i].button_text,objects[i].description);
                //button_num = button_create(buf, objects[i].uid);
                button_create(buf, objects[i].uid);
                printf("button_create(%s) for object[%d].uid=%d\n",buf,i,objects[i].uid);
            };
        }
    }
}

// displays or hides buttons for nearby objects on map
int display_buttons_for_objects (void) {
    float distance;
    for (int o=0;o<num_objects;o++) {
        if (objects[o].valid) {
            distance = get_distance(objects[o].x, objects[o].y, pl.x, pl.y);
            if ((distance < objects[o].size)
                   && (objects[o].owner_num==map.map_number)
                   && (objects[o].owner=='M')) {
                printf("You see: %s\n",objects[o].description);
                // enable buttons
                enable_disable_buttons_by_uid (objects[o].uid, 1);
            } else {
                // object is too far away, disable any buttons
                enable_disable_buttons_by_uid (objects[o].uid, 0);
            }
        } else {
            // button is not valid.
            enable_disable_buttons_by_uid (objects[o].uid, 0);
        }
        // TODO: Manage buttons for objects
    }
    return (1);
}

