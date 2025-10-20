/*
 *
 *
 *


 */

//-----------------------------------------------------------------------------
#ifndef __MONITORMU_SERIAL_H__
#define __MONITORMU_SERIAL_H__

#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>
#include <stdatomic.h>

#include <SDL2/SDL.h>

#include "basic.h"

// ----------------------------------------------------------------------------

typedef struct serial_context_t
{
    const char *dev_p;
    int baud;
    int fd;

    CHANNELS_DATA_TYPE data[MAX_CHANNELS];

    SDL_mutex *lock_p;
    SDL_sem *sem_p;

} serial_context_t;

// ----------------------------------------------------------------------------

int open_serial(serial_context_t *ser_p);

int serial_thread(void*);

void serial_on_sigint(int s);

#endif
