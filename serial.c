/*
 *
 *
 *
 *
 *
 */

#include "serial.h"

// ----------------------------------------------------------------------------

static speed_t
baud_to_flag(int baud)
{
  switch (baud)
  {
    case 9600:
      return B9600;
    case 19200:
      return B19200;
    case 38400:
      return B38400;
    case 57600:
      return B57600;
    case 115200:
      return B115200;
    case 230400:
      return B230400;
    case 460800:
      return B460800;
    case 921600:
      return B921600;
    default:
      return B115200;
  }
}

// ----------------------------------------------------------------------------

int
open_serial(serial_context_t *ser_p)
{
  ser_p->fd = open(ser_p->dev_p, O_RDWR | O_NOCTTY | O_SYNC);
  if (ser_p->fd < 0)
  {
    perror("open serial");
    return -1;
  }

  struct termios tio;
  memset(&tio, 0, sizeof(tio));
  if (tcgetattr(ser_p->fd, &tio) != 0)
  {
    perror("tcgetattr");
    close(ser_p->fd);
    return -1;
  }

  cfmakeraw(&tio);
  speed_t sp = baud_to_flag(ser_p->baud);
  cfsetispeed(&tio, sp);
  cfsetospeed(&tio, sp);

  tio.c_cflag |= (CLOCAL | CREAD);
  tio.c_cflag &= ~CSIZE;
  tio.c_cflag |= CS8;           // 8 data bits
  tio.c_cflag &= ~PARENB;       // no parity
  tio.c_cflag &= ~CSTOPB;       // 1 stop bit

  tio.c_cc[VMIN] = 0;          // non-blocking with timeout
  tio.c_cc[VTIME] = 1;          // 0.1s

  if (tcsetattr(ser_p->fd, TCSANOW, &tio) != 0)
  {
    perror("tcsetattr");
    close(ser_p->fd);
    return -1;
  }
  tcflush(ser_p->fd, TCIFLUSH);

  return ser_p->fd;
}

// ----------------------------------------------------------------------------

extern _Atomic (size_t) stop_flag;

int
serial_thread(void *ctx_p)
{
  serial_context_t *A = (serial_context_t*) ctx_p;

  while (!atomic_load(&stop_flag))
  {
    enum
    {
      SYNC0, SYNC1, DATA
    } st = SYNC0;

    // temporary array
    uint8_t buf[512];
    int di = 0;
    uint8_t frame[sizeof(CHANNELS_DATA_TYPE) * MAX_CHANNELS]; // 7 * float

    int n = read(A->fd, buf, sizeof(buf));
    if (n > 0)
    {
      for (int i = 0; i < n; ++i)
      {
        uint8_t b = buf[i];

        switch (st)
        {
          case SYNC0:
            st = (b == 0x55) ? SYNC1 : SYNC0;
            break;

          case SYNC1:
            if (b == 0xAA)
            {
              st = DATA;
              di = 0;
            }
            else
              st = (b == 0x55) ? SYNC1 : SYNC0; // allow overlapping 0x55
            break;

          case DATA:
            frame[di++] = b;

            // everything (?) is read
            if( di == (sizeof(CHANNELS_DATA_TYPE) * MAX_CHANNELS) )
            {
              SDL_LockMutex(A->lock_p);
              for (int k = 0; k < MAX_CHANNELS; ++k)
              {
                union
                {
                  uint8_t b[sizeof(CHANNELS_DATA_TYPE)];
                  CHANNELS_DATA_TYPE f;
                } u;

                // reload buffer // We do not have much data,
                // #TODO Use buffers switching in case of "bigdata"
                for (uint16_t p = 0; p < sizeof(CHANNELS_DATA_TYPE); p++)
                {
                  u.b[p] = frame[sizeof(CHANNELS_DATA_TYPE) * k + p];
                }

                A->data[k] = u.f;
              }
              SDL_UnlockMutex(A->lock_p);

              SDL_SemPost(A->sem_p);

              st = SYNC0; // reset state machine
            }
            break;
        }
      }
    }
    else if (n == 0)
    {
      // timeout -> loop
    }
    else
    {
      if (errno == EAGAIN || errno == EWOULDBLOCK)
      {
        continue;
      }
      perror("serial read");
      break;
    }
  }

  return 0;
}

// ----------------------------------------------------------------------------
