/*
 * mainwindow.c
 *
 *  Created on: Oct 10, 2025
 *      Author: es
 */

#include "mainwindow.h"

#include "text_resources.h"
#include "basic.h"

static main_window_data_t *win_handle;

_Atomic (size_t) stop_flag;

static const SDL_Color CH_COL[] =
{
  MAIN_WINDOW_COLOUR_CH0,
  MAIN_WINDOW_COLOUR_CH1,
  MAIN_WINDOW_COLOUR_CH2,
  MAIN_WINDOW_COLOUR_CH3,
  MAIN_WINDOW_COLOUR_CH4,
  MAIN_WINDOW_COLOUR_CH5,
  MAIN_WINDOW_COLOUR_CH6
};

static const char *mainwindow_caption = MAIN_WINDOW_CAPTION;
static const char *channels_captions[] =
{
  CHANNEL0_CAPTION,
  CHANNEL1_CAPTION,
  CHANNEL2_CAPTION,
  CHANNEL3_CAPTION,
  CHANNEL4_CAPTION,
  CHANNEL5_CAPTION,
  CHANNEL6_CAPTION
};

static SDL_Color colour_graph_background = MAIN_WINDOW_CHANNEL_BG;
static SDL_Color colour_graph_frame = MAIN_WINDOW_CHANNEL_FRAME;

static CHANNELS_DATA_TYPE data_momentum[MAX_CHANNELS] = { 0 };

// ----------------------------------------------------------------------------

static void draw_text(main_window_data_t *win_p, int x, int y, const char *txt,
    SDL_Color c)
{
  if (!win_p->font_p)
    return;

  SDL_Surface *s = TTF_RenderUTF8_Blended(win_p->font_p, txt, c);
  if (!s)
    return;

  SDL_Texture *t = SDL_CreateTextureFromSurface(win_p->render_p, s);
  SDL_Rect r =
  { x, y, s->w, s->h };
  SDL_FreeSurface(s);
  SDL_RenderCopy(win_p->render_p, t, NULL, &r);
  SDL_DestroyTexture(t);
}

// ----------------------------------------------------------------------------

void main_updatescreen(main_window_data_t *win_p)
{
  SDL_SetRenderDrawColor(win_p->render_p, win_p->color_background.r,
      win_p->color_background.g, win_p->color_background.b,
      win_p->color_background.a);

  SDL_RenderClear(win_p->render_p);

  int w, h;
  SDL_GetWindowSize(win_p->window_p, &w, &h);

// #TODO replace numbers with meaningful defs
  for (int i = 0; i < 4; i++)
  {
    channel_update_area(&win_p->channels_data[i], 8, 8 + i * (8 + h / 4 - 8),
        w / 2 - 2 * 8, h / 4 - 16);
  }
  for (int i = 0; i < 3; i++)
  {
    channel_update_area(&win_p->channels_data[i + 4], w / 2,
        8 + i * (8 + h / 4 - 8), w / 2 - 2 * 8, h / 4 - 16);
  }

  for (int i = 0; i < MAX_CHANNELS; i++)
  {
    channel_draw(&win_p->channels_data[i], win_p->render_p);

    draw_text(win_p, win_p->channels_data[i].x + 4,
        win_p->channels_data[i].y + 4, win_p->channels_data[i].caption,
        win_p->channels_data[i].colour_data);

    char buf[16] =
    { 0 };
    sprintf(buf, "%0.02f", data_momentum[i]);

    draw_text(win_p, win_p->channels_data[i].x + 4,
        win_p->channels_data[i].y + 16, buf,
        win_p->channels_data[i].colour_data);
  }
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

void main_events(main_window_data_t *win_p)
{
  SDL_Event e;

  while (SDL_PollEvent(&e))
  {
    if (e.type == SDL_QUIT)
    {
      atomic_store(&stop_flag, TRUE);
    }
    else if (e.type == SDL_KEYDOWN)
    {
      SDL_Keycode key = e.key.keysym.sym;

      if (key == SDLK_q || key == SDLK_ESCAPE)
      {
        atomic_store(&stop_flag, TRUE);
      }
    }
  }
}

// ----------------------------------------------------------------------------

void main_loop(main_window_data_t *win_p)
{
  while (!atomic_load(&stop_flag))
  {
    main_events(win_p);

    SDL_GetRendererOutputSize(win_p->render_p, &win_p->render_w,
        &win_p->render_h);

    if (SDL_SemTryWait(win_p->serial.sem_p) == 0)
    {
      SDL_LockMutex(win_p->serial.lock_p);

//#define DO_TEST_INCOMING_DATA
#ifdef DO_TEST_INCOMING_DATA
        fprintf(stdout, "%f, %f, %f, %f, %f, %f, %f\r\n",
            win_p->serial.data[0], win_p->serial.data[1],
            win_p->serial.data[2], win_p->serial.data[3],
            win_p->serial.data[4], win_p->serial.data[5],
            win_p->serial.data[6]);
        fflush(stdout);
#endif

      // Do it faster or another way
      for (int i = 0; i < MAX_CHANNELS; i++)
      {
        channel_update(&win_p->channels_data[i], win_p->serial.data[i]);
        data_momentum[i] = win_p->serial.data[i];
      }
      SDL_UnlockMutex(win_p->serial.lock_p);
    }

#ifdef CALIBRATION
    // If calibration is turned on
    for (int i = 0; i < MAX_CHANNELS; i++)
    {
      int pos = win_p->channels_data[i].head - 1;
      if(win_p->channels_data[i].head == 0) pos = MAX_DATA_BUFFERING - 1;

      if(CHANNEL_CALIBRATION_ON == win_p->channels_data[i].cal)
      {
        if(win_p->channels_data[i].cal_samples)
        {
          win_p->channels_data[i].cal_acc += win_p->channels_data[i].data[pos];
          win_p->channels_data[i].cal_samples--;
        }
        else
        {
          win_p->channels_data[i].cal = CHANNEL_CALIBRATION_OFF;
          win_p->channels_data[i].cal_value =
              win_p->channels_data[i].cal_acc / win_p->channels_data[i].cal_order;

          //fprintf(stdout, "Calibr %f\r\n", win_p->channels_data[0].cal_value);
          //fflush(stdout);
        }
      }
      else
      {
        win_p->channels_data[i].data[pos] -= win_p->channels_data[i].cal_value;
        data_momentum[i] = win_p->channels_data[i].data[pos];
      }
    }
#endif

    // update data from buffer
    main_updatescreen(win_p);

    SDL_RenderPresent(win_p->render_p);
    usleep(10);
  }
}

// ----------------------------------------------------------------------------

void main_cleanup(main_window_data_t *win_p)
{
  int thread_ret = 0;
  SDL_WaitThread(win_p->thread_serial_p, &thread_ret);

// #TODO Close log file here if it is open

  close(win_p->serial.fd);

  SDL_DestroySemaphore(win_p->serial.sem_p);
  SDL_DestroyMutex(win_p->serial.lock_p);
  SDL_DestroyRenderer(win_p->render_p);
  SDL_DestroyWindow(win_p->window_p);
  SDL_Quit();
}


// #TODO review closing conditions in case of errors

int main_create(main_window_data_t *win_p)
{
  int res = 1;

  do
  {
    atomic_store(&stop_flag, 0);

    if (NULL == win_p)
    {
      break;
    }

    win_handle = win_p;

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_EVENTS) != 0)
    {
      fprintf(stderr, "SDL_Init error: %s\n", SDL_GetError());
      break;
    }

    if (TTF_Init() != 0)
    {
      fprintf(stderr, "TTF_Init: %s\n", TTF_GetError());
    }

    win_p->caption = mainwindow_caption;

    // Load a font (what's available?)
    win_p->font_p = TTF_OpenFont(
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", 14);
    if (!win_p->font_p)
    {
      fprintf(stderr, "Font load error\n");
      break;
    }

    // ------------------------------------------------------------------------

    win_p->window_p = SDL_CreateWindow(win_p->caption,
    SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, win_p->win_w, win_p->win_h,
        SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN);

    if (!win_p->window_p)
    {
      fprintf(stderr, "SDL_CreateWindow error: %s\n", SDL_GetError());
      SDL_Quit();
      break;
    }

    win_p->render_p = SDL_CreateRenderer(win_p->window_p, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!win_p->render_p)
    {
      fprintf(stderr, "SDL_CreateRenderer error: %s\n", SDL_GetError());
      SDL_DestroyWindow(win_p->window_p);
      SDL_Quit();
      break;
    }

    // ------------------------------------------------------------------------

    win_p->serial.lock_p = SDL_CreateMutex();
    if (!win_p->serial.lock_p)
    {
      fprintf(stderr, "SDL mutex error\n");
      break;
    }

    win_p->serial.sem_p = SDL_CreateSemaphore(0);
    if (!win_p->serial.sem_p)
    {
      fprintf(stderr, "SDL semaphore error\n");
      break;
    }

    // ------------------------------------------------------------------------

    int fd = open_serial(&win_p->serial);
    if (fd < 0)
    {
      fprintf(stderr, "Serial port error\n");
      break;
    }

    win_p->thread_serial_p = SDL_CreateThread(serial_thread,
        "abox_serial_thread", &win_p->serial);

    if (!win_p->thread_serial_p)
    {
      fprintf(stderr, "SDL_CreateThread error: %s\n", SDL_GetError());
      break;
    }

    // Init channels data
    int w, h;
    SDL_GetWindowSize(win_p->window_p, &w, &h);

    // #TODO Replace number with meaningful defs,
    // actually that are coords, but ordnung muss sein
    for (int i = 0; i < 4; i++)
    {
      channel_init(&win_p->channels_data[i], channels_captions[i], 8,
          8 + i * (8 + h / 4 - 8), w / 2 - 2 * 8, h / 4 - 16, CH_COL[i],
          colour_graph_background, colour_graph_frame);
    }
    for (int i = 0; i < 3; i++)
    {
      channel_init(&win_p->channels_data[i + 4], channels_captions[i + 4],
          w / 2, 8 + i * (8 + h / 4 - 8), w / 2 - 8, h / 4 - 16, CH_COL[i + 4],
          colour_graph_background, colour_graph_frame);
    }

#ifdef CALIBRATION
    for (int i = 0; i < MAX_CHANNELS; i++)
    {
      win_p->channels_data[i].cal = CHANNEL_CALIBRATION_ON;
      win_p->channels_data[i].cal_acc = 0;
      win_p->channels_data[i].cal_order = 250;
      win_p->channels_data[i].cal_samples = win_p->channels_data[i].cal_order;
    }
#endif

    signal(SIGINT, serial_on_sigint);
    res = 0;
  } while (0);

  return res;
}

void serial_on_sigint(int s)
{
  (void) s;

  atomic_store(&stop_flag, TRUE);
}
