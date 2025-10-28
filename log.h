/*
 * log.h
 *
 *  Created on: Oct 26, 2025
 *      Author: es
 */

#ifndef __MONITORMU_LOG_H__
#define __MONITORMU_LOG_H__

#  include <time.h>
#  include <stdio.h>
#  include <string.h>

#  include "basic.h"

// ----------------------------------------------------------------------------

#  define LOG_MAX_FILENAME_LENGTH 256

typedef struct
{
  char file_name[LOG_MAX_FILENAME_LENGTH];
  FILE * file_p;

}log_t;

// ----------------------------------------------------------------------------

void log_init(log_t *l_p);

int log_mu(log_t *, CHANNELS_DATA_TYPE *, int);

#endif
