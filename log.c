/*
 * log.c
 *
 *  Created on: Oct 28, 2025
 *      Author: es
 */

#include "log.h"

// ----------------------------------------------------------------------------

void log_init(log_t *l_p)
{
  memset(l_p->file_name, 0, sizeof(l_p->file_name));

  time_t now = time(NULL);
  struct tm tm_now;
  localtime_r(&now, &tm_now);

  strftime(l_p->file_name, sizeof(l_p->file_name), "log_%Y%m%d-%H%M%S.csv", &tm_now);
}

// ----------------------------------------------------------------------------

int log_mu(log_t *l_p, CHANNELS_DATA_TYPE * d_p, int n)
{
  l_p->file_p = fopen(l_p->file_name, "a");

  if(l_p->file_p)
  {
    for (int i = 0; i < n; ++i)
    {

      // #TODO Play around the data type here
//#define DATA_TYPE float

//#if CHANNELS_DATA_TYPE == DATA_TYPE
        fprintf(l_p->file_p, "%.6f", d_p[i]);
//#endif
        if (i < n - 1)
            fputc(',', l_p->file_p);
    }
    fprintf(l_p->file_p, "\r\n");

    fclose(l_p->file_p);

    return 0;
  }

  return 1;
}
