/*
 * basic.h
 *
 *  Created on: Oct 10, 2025
 *      Author: es
 */

#ifndef __BASIC_H__
#  define __BASIC_H__


#  define TRUE                          1
#  define FALSE                         0

// ----------------------------------------------------------------------------

typedef enum
{
  MM_OK = 0,
  MM_SERIAL_ERROR,

}mm_error_t;

// ----------------------------------------------------------------------------

// channels definitions
#  define MAX_CHANNELS                  7
#  define MAX_DATA_BUFFERING            512
#  define CHANNELS_DATA_TYPE            float

// main windows properties
#  define MAIN_WINDOW_COLOUR_BACKGROUND { 0, 0, 20, 255 }

// Channels options
#  define MAIN_WINDOW_COLOUR_CH0        { 80, 180, 255, 255 }
#  define MAIN_WINDOW_COLOUR_CH1        { 255, 120, 120, 255 }
#  define MAIN_WINDOW_COLOUR_CH2        { 120, 255, 140, 255 }
#  define MAIN_WINDOW_COLOUR_CH3        { 255, 210, 80, 255 }
#  define MAIN_WINDOW_COLOUR_CH4        { 200, 120, 255, 255 }
#  define MAIN_WINDOW_COLOUR_CH5        { 120, 220, 220, 255 }
#  define MAIN_WINDOW_COLOUR_CH6        { 220, 220, 220, 255 }

#  define MAIN_WINDOW_CHANNEL_BG        { 20, 20, 20, 255 }
#  define MAIN_WINDOW_CHANNEL_FRAME     { 40, 40, 40, 255 }

#  define MAIN_WINDOW_ERROR_DELAY       2

// channels areas coords
#  define CHANNEL_CAPTION_XOFFSET   4
#  define CHANNEL_CAPTION_YOFFSET   4

#  define CHANNEL_VALUE_XOFFSET     4
#  define CHANNEL_VALUE_YOFFSET     24

#  define CHANNEL_AREA_XOFFSET      8
#  define CHANNEL_AREA_YOFFSET      8

#  define CHANNELS_IN_COLUMN        4


#endif
