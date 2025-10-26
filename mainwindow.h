/*
 * mainwindow.h
 *
 *  Created on: Oct 10, 2025
 *      Author: es
 */

#ifndef __MAINWINDOW_H__
#define __MAINWINDOW_H__

#  include <stdio.h>
#  include <errno.h>
#  include <fcntl.h>
#  include <signal.h>
#  include <stdio.h>
#  include <stdlib.h>
#  include <stdint.h>
#  include <string.h>
#  include <termios.h>
#  include <time.h>
#  include <unistd.h>
#  include <stdatomic.h>

#  include <SDL2/SDL.h>
#  include <SDL2/SDL_ttf.h>

#  include "basic.h"

#  include "serial.h"
#  include "channel.h"

// ----------------------------------------------------------------------------

typedef struct
{
    int render_w, render_h;
    SDL_Window *window_p;
    SDL_Renderer *render_p;
    TTF_Font *font_p;

    SDL_Color color_background;

    int win_w, win_h;
    const char *caption;

    serial_context_t serial;
    SDL_Thread *thread_serial_p;

    channel_t channels_data[MAX_CHANNELS];

    int err;

} main_window_data_t;

// ----------------------------------------------------------------------------

void main_displayerror(main_window_data_t *, mm_error_t);
mm_error_t main_create(main_window_data_t*);
void main_events(main_window_data_t*);
void main_updatescreen(main_window_data_t*);
void main_loop(main_window_data_t*);
void main_cleanup(main_window_data_t*);

// ----------------------------------------------------------------------------

#endif
