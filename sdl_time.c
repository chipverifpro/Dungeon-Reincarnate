// sdl_time.c

#include "all_headers.h"

// GLOBAL variables
int game_time = 0;
int animation_time = 20; //ms
SDL_TimerID timerID;

void send_user_event(int event_code) {
    SDL_Event event;

    event.type = SDL_USEREVENT;
    event.user.code = event_code;
    event.user.data1 = 0;
    event.user.data2 = 0;
    SDL_PushEvent(&event);
};

Uint32 callback(Uint32 interval, void* name) {
    game_time++;
    send_user_event(1); // periodic wakeup to GUI loop to allow processing animations.
    
    return interval;
}

int setup_callback_interval (int interval_ms) {
    timerID = SDL_AddTimer(interval_ms, callback, "SDL");

    printf("Started timer every %dms\n",interval_ms);
    return ((int) timerID);
}

int remove_callback_interval (void) {
    int result;
    result = SDL_RemoveTimer (timerID);
    return result;
}

void print_statistics(void) {
    int minutes, hours;
    float seconds;
    printf("Number of screen renders    : %d\n",num_times_rendered);
    printf("Number of events            : %d\n",num_events_observed);
    printf("Number of events_user       : %d\n",num_events_user);
    printf("Number of events_window     : %d\n",num_events_window);
    printf("Number of events_keyboard   : %d\n",num_events_keyboard);
    printf("Number of events_mouse      : %d\n",num_events_mouse);
    printf("Number of gui button clicks : %d\n",num_events_button);
    printf("Number of player steps      : %d\n",num_events_player_step);
    printf("Number of animation events  : %d\n",num_events_animation);
    seconds = game_time*animation_time/1000.0;
    minutes = ifloor(seconds/60.0);
    hours   = ifloor(minutes/60.0);
    if (minutes>=1) {
        printf("Game run time (%d:%02d:%0.3f) : %0.3f s\n",hours, minutes-hours*60, seconds - minutes*60, seconds);
    } else {
        printf("Game run time               : %0.3f s\n",game_time*animation_time/1000.0);
    }
    time(&end_time);
    seconds = end_time-start_time;
    minutes = ifloor(seconds/60.0);
    hours   = ifloor(minutes/60.0);
    if (minutes>=1) {
        printf("Clock time    (%d:%02d:%0.3f) : %0.3f s\n",hours, minutes-hours*60, seconds - minutes*60, seconds);
    } else {
        printf("Clock time                  : %ld s\n",end_time-start_time);
    };
}
