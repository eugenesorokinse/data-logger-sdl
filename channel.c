/*
 * channel.c
 *
 *  Created on: Oct 11, 2025
 *      Author: es
 */

#include "channel.h"

#ifndef CHANNELS_DATA_TYPE
#  define CHANNELS_DATA_TYPE float
#endif

// ----------------------------------------------------------------------------

#ifndef CHANNELS_TO_FLOAT
#  define CHANNELS_TO_FLOAT(v) ((float)(v))
#endif

#ifndef CHANNELS_SCALE
#  define CHANNELS_SCALE  (1.0f)
#endif

#ifndef CHANNELS_OFFSET
#  define CHANNELS_OFFSET (0.0f)
#endif

static inline float chan_to_float(CHANNELS_DATA_TYPE v)
{
  return CHANNELS_TO_FLOAT(v) * CHANNELS_SCALE + CHANNELS_OFFSET;
}

static void wf_compute_minmax(const CHANNELS_DATA_TYPE *v, size_t n,
    float *out_min, float *out_max)
{
  float mn = FLT_MAX, mx = -FLT_MAX;
  for (size_t i = 0; i < n; ++i)
  {
    float x = chan_to_float(v[i]);
    if (isnan(x))
      continue;
    if (x < mn)
      mn = x;
    if (x > mx)
      mx = x;
  }
  if (mn == FLT_MAX)
  {
    mn = -1.f;
    mx = 1.f;
  }           // all NaN fallback
  if (mn == mx)
  {
    float eps = (mn == 0.f) ? 1.f : fabsf(mn) * 0.05f;
    mn -= eps;
    mx += eps;
  }
  *out_min = mn;
  *out_max = mx;
}

static inline int wf_map_y(float v, float mn, float mx, int y, int h)
{
  if (mx == mn)
    mx = mn + 1.f;
  float t = (v - mn) / (mx - mn);       // 0..1 bottom->top
  if (t < 0.f)
    t = 0.f;
  else if (t > 1.f)
    t = 1.f;
  return y + (int) lroundf((1.f - t) * (float) (h - 1));
}

static inline void draw_waveform_channel(SDL_Renderer *ren,
    const CHANNELS_DATA_TYPE *data, size_t n, int x, int y, int w, int h,
    SDL_Color color, int autoscale, float ymin, float ymax)
{
  if (!ren || !data || n == 0 || w <= 1 || h <= 1)
    return;

#if 1
  // #TODO Bring custom colours from settings
  // Background + border (optional)
  SDL_SetRenderDrawColor(ren, 10, 10, 20, 255);
  SDL_Rect r = { x, y, w, h };
  SDL_RenderFillRect(ren, &r);
  SDL_SetRenderDrawColor(ren, 120, 120, 120, 255);
  SDL_RenderDrawRect(ren, &r);

  // Y range
  float mn = ymin, mx = ymax;

  if (autoscale)
  {
    wf_compute_minmax(data, n, &mn, &mx);
    float pad = (mx - mn) * 0.05f;
    if (pad == 0.f)
      pad = 1.f;
    mn -= pad;
    mx += pad;
  }
  if (mx == mn)
  {
    mx += 1.f;
    mn -= 1.f;
  }

  // Zero line if visible
  if (mn < 0.f && mx > 0.f)
  {
    int zy = wf_map_y(0.f, mn, mx, y, h);
    SDL_SetRenderDrawColor(ren, 90, 90, 110, 255);
    SDL_RenderDrawLine(ren, x, zy, x + w - 1, zy);
  }

  SDL_SetRenderDrawColor(ren, color.r, color.g, color.b, color.a);

  if (n <= (size_t) w)
  {
    // Polyline (upsample or 1:1)
    float sx = (float) (w - 1) / (float) ((n > 1) ? (n - 1) : 1);
    int px0 = x;
    int py0 = wf_map_y(chan_to_float(data[0]), mn, mx, y, h);
    for (size_t i = 1; i < n; ++i)
    {
      int px1 = x + (int) lroundf((float) i * sx);
      int py1 = wf_map_y(chan_to_float(data[i]), mn, mx, y, h);
      SDL_RenderDrawLine(ren, px0, py0, px1, py1);
      px0 = px1;
      py0 = py1;
    }
  }
  else
  {
    // Min–max binning per column
    float spp = (float) n / (float) w;  // samples per pixel
    for (int col = 0; col < w; ++col)
    {
      size_t start = (size_t) floorf((float) col * spp);
      size_t end = (size_t) floorf((float) (col + 1) * spp);
      if (end <= start)
        end = start + 1;
      if (end > n)
        end = n;

      float cmin = FLT_MAX, cmax = -FLT_MAX;
      for (size_t k = start; k < end; ++k)
      {
        float v = chan_to_float(data[k]);
        if (isnan(v))
          continue;
        if (v < cmin)
          cmin = v;
        if (v > cmax)
          cmax = v;
      }
      if (cmin == FLT_MAX)
        continue; // bin was all NaN
      int y1 = wf_map_y(cmin, mn, mx, y, h);
      int y2 = wf_map_y(cmax, mn, mx, y, h);
      int px = x + col;
      SDL_RenderDrawLine(ren, px, y1, px, y2);
    }
  }
#endif
}

// ----------------------------------------------------------------------------

void channel_update(channel_t *p, CHANNELS_DATA_TYPE d)
{
  p->data[p->head++] = d;

  if (p->head >= MAX_DATA_BUFFERING)
    p->head = 0;
}

// ----------------------------------------------------------------------------

void channel_draw(channel_t *p, SDL_Renderer *r)
{
  draw_waveform_channel(r, p->data, MAX_DATA_BUFFERING, p->x, p->y, p->w, p->h,
      p->colour_data, TRUE, 0, 0);

}

void channel_init(channel_t *p, const char *caption, int x, int y, int w, int h,
    SDL_Color cd, SDL_Color cb, SDL_Color cf)
{
  p->caption = (const char*) caption;
  p->x = x;
  p->y = y;
  p->w = w;
  p->h = h;
  p->colour_data = cd;
  p->colour_background = cb;
  p->colour_frame = cf;
}

// ----------------------------------------------------------------------------

void channel_update_area(channel_t *p, int x, int y, int w, int h)
{
  p->x = x;
  p->y = y;
  p->w = w;
  p->h = h;
}
