/*
 * mainwindow.c
 *
 *  Created on: Oct 10, 2025
 *      Author: es
 */

// #TODO rework calibration features
// #TODO add easy-to-implement math channels features

#include "mainwindow.h"

#include "text_resources.h"
#include "basic.h"

// ----------------------------------------------------------------------------

_Atomic (size_t) stop_flag;

//#TODO Log flag may be not atomical
_Atomic (size_t) log_flag;

// ----------------------------------------------------------------------------

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

// ----------------------------------------------------------------------------

// Default channels' colours
static SDL_Color colour_graph_background = MAIN_WINDOW_CHANNEL_BG;
static SDL_Color colour_graph_frame = MAIN_WINDOW_CHANNEL_FRAME;

// Momentum data to visualize
static CHANNELS_DATA_TYPE data_momentum[MAX_CHANNELS] = { 0 };

// ----------------------------------------------------------------------------

static void
draw_text(main_window_data_t *win_p, int x, int y, const char *txt,
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

void
main_displayerror(main_window_data_t *win_p, mm_error_t err)
{
  SDL_SetRenderDrawColor(win_p->render_p,
      win_p->color_background.r,
      win_p->color_background.g,
      win_p->color_background.b,
      win_p->color_background.a);

  SDL_RenderClear(win_p->render_p);

  int w, h;
  SDL_GetWindowSize(win_p->window_p, &w, &h);

  // #TODO Get text width and center text's position
  switch (err)
  {
    case (MM_SERIAL_ERROR):
    {
      SDL_Colour colour = { 255, 255, 255, 255 };
      draw_text(win_p, 200, 100, MSG_ERROR_SERIALPORTOPENNING, colour);
    }
      break;

    default:
    {
      SDL_Colour colour = { 255, 0, 0, 255 };
      draw_text(win_p, 200, 100, MSG_ERROR_GENERAL, colour);
    }
    break;
  }

  SDL_RenderPresent(win_p->render_p);
  sleep(MAIN_WINDOW_ERROR_DELAY);
}

// ----------------------------------------------------------------------------

void
main_updatescreen(main_window_data_t *win_p)
{
  SDL_SetRenderDrawColor(win_p->render_p, win_p->color_background.r,
      win_p->color_background.g, win_p->color_background.b,
      win_p->color_background.a);

  SDL_RenderClear(win_p->render_p);

  int w, h;
  SDL_GetWindowSize(win_p->window_p, &w, &h);

  // --------------------------------------------------------------------------

  // update channels' areas in case if window's geometry has been changed
  // a bit messy and not tested in configuration other
  // that 2 columns with 4 lines
  for (int i = 0; i < CHANNELS_IN_COLUMN; i++)
  {
    channel_update_area(&win_p->channels_data[i],
        CHANNEL_AREA_XOFFSET,
        CHANNEL_AREA_YOFFSET + i * (h / CHANNELS_IN_COLUMN),
        w / CHANNELS_COLUMNS - CHANNELS_COLUMNS * CHANNEL_AREA_XOFFSET,
        h / CHANNELS_IN_COLUMN - (CHANNELS_COLUMNS - 1) * CHANNEL_AREA_YOFFSET);
  }
  for (int i = 0; i < (MAX_CHANNELS - CHANNELS_IN_COLUMN); i++)
  {
    channel_update_area(&win_p->channels_data[i + CHANNELS_IN_COLUMN],
        w / CHANNELS_COLUMNS,
        CHANNEL_AREA_YOFFSET + i * (h / CHANNELS_IN_COLUMN),
        w / CHANNELS_COLUMNS - (CHANNELS_COLUMNS - 1) * CHANNEL_AREA_XOFFSET,
        h / CHANNELS_IN_COLUMN - (CHANNELS_COLUMNS - 1) * CHANNEL_AREA_YOFFSET);
  }

  // --------------------------------------------------------------------------

  // redraw channels' caption/momentum_values/data
  for (int i = 0; i < MAX_CHANNELS; i++)
  {
    channel_draw(&win_p->channels_data[i], win_p->render_p);

    draw_text(win_p, win_p->channels_data[i].x + CHANNEL_CAPTION_XOFFSET,
        win_p->channels_data[i].y + CHANNEL_CAPTION_YOFFSET,
        win_p->channels_data[i].caption, win_p->channels_data[i].colour_data);

    char buf[16] = { 0 };
    sprintf(buf, "%0.02f", data_momentum[i]);

    draw_text(win_p, win_p->channels_data[i].x + CHANNEL_VALUE_XOFFSET,
        win_p->channels_data[i].y + CHANNEL_VALUE_YOFFSET, buf,
        win_p->channels_data[i].colour_data);
  }

  SDL_RenderPresent(win_p->render_p);
}

// ----------------------------------------------------------------------------
// #TODO compilation warning will be mitigated, pointer will be used i framtiden

void
main_events(main_window_data_t *win_p)
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

      // Quit
      if (key == SDLK_q || key == SDLK_ESCAPE)
      {
        atomic_store(&stop_flag, TRUE);
      }

      // Logging toggle
      if (key == SDLK_s)
      {
        size_t log_state = atomic_load(&log_flag);
        if(FALSE == log_state)
        {
          log_init(&win_p->log);
          log_state = TRUE;
        }
        else
        {
          log_state = FALSE;
        }
        atomic_store(&log_flag, log_state);
        fflush(stdout);
      }
    }
  }
}

// ----------------------------------------------------------------------------

void
main_loop(main_window_data_t *win_p)
{
  while (!atomic_load(&stop_flag))
  {
    main_events(win_p);

    SDL_GetRendererOutputSize(win_p->render_p, &win_p->render_w,
        &win_p->render_h);

    if (SDL_SemTryWait(win_p->serial.sem_p) == 0)
    {
      SDL_LockMutex(win_p->serial.lock_p);

      // Do it faster or another way
      // #TODO shift to doublebuffering mmmm.... but we have quite plenty of time
      for (int i = 0; i < MAX_CHANNELS; i++)
      {
        channel_update(&win_p->channels_data[i], win_p->serial.data[i]);
        data_momentum[i] = win_p->serial.data[i];
      }
      SDL_UnlockMutex(win_p->serial.lock_p);

      if(TRUE == atomic_load(&log_flag))
      {
        log_mu(&win_p->log, data_momentum, MAX_CHANNELS);
      }
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
    usleep(10);
  }
}

// ----------------------------------------------------------------------------

void
main_cleanup(main_window_data_t *win_p)
{
  int thread_ret = 0;
  SDL_WaitThread(win_p->thread_serial_p, &thread_ret);

// #TODO Close log file here if it is open

  close(win_p->serial.fd);

  if(NULL != win_p->serial.sem_p)SDL_DestroySemaphore(win_p->serial.sem_p);
  if(NULL != win_p->serial.lock_p)SDL_DestroyMutex(win_p->serial.lock_p);
  if(NULL != win_p->render_p)SDL_DestroyRenderer(win_p->render_p);
  if(NULL != win_p->window_p)SDL_DestroyWindow(win_p->window_p);
  SDL_Quit();
}

// #TODO review closing conditions in case of errors
// SDL cleaned up in main_cleanup() call in main()

mm_error_t
main_create(main_window_data_t *win_p)
{
  int res = 1;

  do
  {
    atomic_store(&stop_flag, 0);

    if (NULL == win_p)
    {
      break;
    }

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
    // #TODO define ttf font path in header, place local copy of
    // font near executable file(?)
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
      break;
    }

    win_p->render_p = SDL_CreateRenderer(win_p->window_p, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!win_p->render_p)
    {
      fprintf(stderr, "SDL_CreateRenderer error: %s\n", SDL_GetError());
      SDL_DestroyWindow(win_p->window_p);
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
      fprintf(stderr, MSG_ERROR_SERIALPORTOPENNING "\n");
      main_displayerror(win_p, MM_SERIAL_ERROR);
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

    // Actually these are channels areas coords, but ordnung muss sein
    // If you have other amount of channels, then place them as you need :)

    for (int i = 0; i < CHANNELS_IN_COLUMN; i++)
    {
      channel_init(&win_p->channels_data[i],
          channels_captions[i],
          CHANNEL_AREA_XOFFSET,
          CHANNEL_AREA_YOFFSET + i * (h / CHANNELS_IN_COLUMN),
          w / CHANNELS_COLUMNS - (CHANNELS_COLUMNS - 1) * CHANNEL_AREA_XOFFSET,
          h / CHANNELS_IN_COLUMN - CHANNELS_COLUMNS * CHANNEL_AREA_YOFFSET,
          CH_COL[i],
          colour_graph_background, colour_graph_frame);
    }

    // initialize the second column of channels's areas
    for (int i = 0; i < (MAX_CHANNELS - CHANNELS_IN_COLUMN); i++)
    {
      channel_init(&win_p->channels_data[i + CHANNELS_IN_COLUMN],
          channels_captions[i + CHANNELS_IN_COLUMN],
          w / CHANNELS_COLUMNS,
          CHANNEL_AREA_YOFFSET + i * (h / CHANNELS_IN_COLUMN),
          w / CHANNELS_COLUMNS,
          h / CHANNELS_IN_COLUMN - (CHANNELS_COLUMNS - 1) * CHANNEL_AREA_YOFFSET,
          CH_COL[i + CHANNELS_IN_COLUMN],
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

// ----------------------------------------------------------------------------

void
serial_on_sigint(int s)
{
  (void) s;

  atomic_store(&stop_flag, TRUE);
}

// ----------------------------------------------------------------------------
