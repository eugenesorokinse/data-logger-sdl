/*
 * channel.h
 *
 *  Created on: Oct 11, 2025
 *      Author: es
 */

#ifndef __MONITORMU_CHANNEL_H__
#define __MONITORMU_CHANNEL_H__

#  include <float.h>
#  include <math.h>

#  include <SDL2/SDL.h>
#  include <SDL2/SDL_ttf.h>

#  include "basic.h"

// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------

typedef enum
{
  CHANNEL_CALIBRATION_OFF = 0,
  CHANNEL_CALIBRATION_ON
}calibrate_flag_t;


typedef struct channel_t
{
    calibrate_flag_t  cal;
    float cal_acc;
    int cal_samples;
    float cal_value;
    float cal_order;

    CHANNELS_DATA_TYPE data[MAX_DATA_BUFFERING];
    int head;

    const char * caption;

    // Visual area properties
    int x, y;
    int w, h;

    SDL_Color colour_data;
    SDL_Color colour_frame;
    SDL_Color colour_background;
    SDL_Color colour_zeroline;

} channel_t;

// ----------------------------------------------------------------------------

void channel_update(channel_t * p, CHANNELS_DATA_TYPE d);
void channel_draw(channel_t * p, SDL_Renderer *r);

void channel_init(channel_t *p,
    const char * caption,
    int x, int y, int w, int h,
    SDL_Color, SDL_Color, SDL_Color);

void channel_update_area(channel_t *p, int x, int y, int w, int h);


#endif
