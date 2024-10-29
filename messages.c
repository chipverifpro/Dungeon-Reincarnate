// messages.c
#include "all_headers.h"

/*
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>

#include "dungeon.h"
#include "buttons.h"
#include "messages.h"
#include "items.h"
#include "sdl_time.h"
#include "sdl_draw.h"
#include "player.h"
#include "map_io.h"
*/

struct button_s *messages = NULL; // list of messages
int *life;

int num_messages=0;               // number of messages in list
int message_font_size=20;         // global font size for messages

int message_border_size = 2;      // pixels added to text each side
int message_spacing = 4;          // space between two messages
int message_from_edge_spacing = 8; // Don't draw messages right at edge

void init_messages(void) {
    messages = malloc(sizeof(struct button_s)*50);
    life = malloc(sizeof(int)*50);
    num_messages = 0;
    for (int i =0;i<50;i++) {
        messages[i].texture=NULL;
        life[i]=0;
    }
}

SDL_Texture *message_create_texture(int message_num) {
    SDL_Surface *message_surface;
    SDL_Texture *message_texture;

    // clear any previously created textures and re-create it.
    //if (messages[message_num].texture!=NULL) {
    //    SDL_DestroyTexture(messages[message_num].texture);
    //    messages[message_num].texture = NULL;
    //}
    // determine width and height of text
    TTF_SizeText(font, messages[message_num].text,
                 &messages[message_num].rect.w,
                 &messages[message_num].rect.h);
    messages[message_num].rect.x = 0;  // message_num manager will override
    messages[message_num].rect.y = 0;
    // render the text to a surface, then copy that to a texture
    // TODO: WRAP TEXT
    int max_message_width = screen_width - 2*message_border_size - 2*message_from_edge_spacing;
    message_surface = TTF_RenderText_Blended_Wrapped(font, messages[message_num].text, messages[message_num].fg_color, max_message_width);
    //message_surface = TTF_RenderText_Blended(font, messages[message_num].text, messages[message_num].fg_color);
    if (message_surface==NULL) {
        printf("TTF_RenderText_Blended_Wrapped FAILED: %s\n",SDL_GetError());
        exit(-1);
    }
    messages[message_num].rect.h = message_surface->h + 2*message_border_size;
    messages[message_num].rect.w = message_surface->w + 2*message_border_size;
    message_texture = SDL_CreateTextureFromSurface(renderer, message_surface);
    if (message_texture==NULL) {
        printf("TTF_CreateTextureFromSurface FAILED: %s\n",SDL_GetError());
        exit(-1);
    }
    SDL_FreeSurface(message_surface);

    return (message_texture);
}

int message_create(char *text, int uid) {
    SDL_Color message_color;
    SDL_Color background_color;
    SDL_Color border_color;

    //printf("message_create: %s\n",text);
    if (messages==NULL) {
        messages=malloc(50 * sizeof(struct button_s));
        //printf("malloc success\n");
    }
    message_color    = color_from_rgba(COLOR_CYAN);
    background_color = color_from_rgba(COLOR_BLACK_TRANSPARENT);
    border_color     = color_from_rgba(COLOR_TRANSPARENT);

    //printf("calling chomp\n");
    chomp(text); // get rid of newline
    messages[num_messages].text = str_alloc_copy(text);
    messages[num_messages].visible = 1;
    messages[num_messages].valid = 1;
    messages[num_messages].object_uid = uid;
    messages[num_messages].fg_color = message_color;
    messages[num_messages].bg_color = background_color;
    messages[num_messages].border_color = border_color;

    //
    messages[num_messages].texture = message_create_texture(num_messages);
    life[num_messages] = 50;

    //printf("Created message %d: %s\n",num_messages,messages[num_messages].text);
    //
    dirty_display = 8;
    num_messages++;
    return (num_messages-1); // return the created message number
}

int message_destroy(int message_num) {
    if (message_num>=num_messages) {
        return(-1);
    };
    if (messages[message_num].texture!=NULL) {
        printf("not destroying texture[%d] %p\n",message_num, messages[message_num].texture);
        //SDL_DestroyTexture(messages[message_num].texture);
        messages[message_num].texture = NULL;
    }
    messages[message_num].valid = 0;
    dirty_display = 8;
    return (0);
}

int draw_one_message(int message_num) {    // render a single message
    SDL_Rect message_rect;
    if ((message_num>=num_messages) || (messages[message_num].valid == 0)) {
        printf("Requested message_num %d not valid\n",message_num);
        return (-1);
    }
    // Draw the Background
    message_rect = messages[message_num].rect;
    SDL_SetRenderDrawColor(renderer, messages[message_num].bg_color.r,
                                   messages[message_num].bg_color.g,
                                   messages[message_num].bg_color.b,
                                   messages[message_num].bg_color.a);
    SDL_RenderFillRect(renderer, &message_rect);
    // Draw the Frame
    SDL_SetRenderDrawColor(renderer, messages[message_num].border_color.r,
                                   messages[message_num].border_color.g,
                                   messages[message_num].border_color.b,
                                   messages[message_num].border_color.a);
    SDL_RenderDrawRect(renderer, &message_rect);
    // Draw the Text Label
    message_rect.x += message_border_size; // shrink rect to text size
    message_rect.y += message_border_size;
    message_rect.h -= 2*message_border_size;
    message_rect.w -= 2*message_border_size;
    //printf("SDL_Render_Copy message_num=%d, texture=%p\n",message_num, messages[message_num].texture);
    SDL_RenderCopy(renderer, messages[message_num].texture, NULL, &message_rect);
    return (message_num);
}

// Upper left corner messages
int arrange_and_draw_messages(void) {     // arrange and draw all active messages
    int number_drawn =0;   // statistics
    int x,y;    // upper left of the current message
    int i;

    x=message_from_edge_spacing;
    y=message_from_edge_spacing;

    for (i=0;i<num_messages;i++) {
        if (messages[i].valid && messages[i].visible) {
            if (life[i] <=0) {
                //printf("Arrange and draw messages: destroy %d\n",i);
                message_destroy(i);
                //printf("Complete\n");
                continue;
            } else {
                life[i]--;
            }
            messages[i].rect.x = x;
            messages[i].rect.y = y;
            draw_one_message(i);
            //life[i]--;
            y+=messages[i].rect.h + 2*message_border_size + message_spacing;
            number_drawn++;
        }
    }

    if (number_drawn>0) {
        //printf("arrange_and_draw_messages drew %d messages.\n",number_drawn);
    }

    return(number_drawn);
}

// functions to look up message number...
int which_message(int scr_x, int scr_y) { // converts screen coordinates to visible message number
    int i;
    SDL_Point scr;
    scr.x = scr_x;  scr.y = scr_y;
    for (i=0;i<num_messages;i++) {
        if ((messages[i].valid == 0) || (messages[i].visible == 0)) {
            continue;
        }
        if (SDL_PointInRect(&scr,&messages[i].rect)) {
            return(i);
        }
    }   
    return(-1);
}
