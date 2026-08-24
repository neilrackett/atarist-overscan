/*
 * Copyright (C) 2026 Neil Rackett
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "demo.h"

#include <stdint.h>

/* Once the borders open the hardware keeps fetching past the visible
 * window (up to 276 rows with both borders removed), so the buffers
 * must be tall enough to cover that. Everything beyond visible_lines
 * is left as zeros, which display as colour 0 - identical to the
 * border, so the padding is invisible. */
#define SCREEN_LINES 280
#define SCREEN_BYTES (160 * SCREEN_LINES)

extern volatile uint32_t scraddr1;
extern volatile uint32_t scraddr2;
extern volatile uint8_t backbuf_flag;

static uint8_t scrbuf[2 * SCREEN_BYTES + 256];
static uint16_t save_pal[16];

static const uint16_t palette[16] = {
    0x0000, /* 0: black (border and checkerboard dark) */
    0x0000,
    0x0000,
    0x0777, /* 3: white (checkerboard light) */
    0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000};

static void save_palette(void)
{
  volatile uint16_t *hw = (volatile uint16_t *)0xffff8240;
  uint16_t i;

  for (i = 0; i < 16; ++i)
  {
    save_pal[i] = hw[i];
  }
}

static void set_palette(const uint16_t *pal)
{
  volatile uint16_t *hw = (volatile uint16_t *)0xffff8240;
  uint16_t i;

  for (i = 0; i < 16; ++i)
  {
    hw[i] = pal[i];
  }
}

static void disable_mouse(void)
{
  *(volatile uint8_t *)0xfffffc02 = 0x12;
}

static void enable_mouse(void)
{
  *(volatile uint8_t *)0xfffffc02 = 0x08;
}

static uint8_t *screen_base(uint8_t *buffer)
{
  uintptr_t base = (uintptr_t)(buffer + 255);
  base &= ~(uintptr_t)0xff;
  return (uint8_t *)base;
}

/*
 * Draw a checkerboard pattern using bitplane 0 and 1.
 * Only visible_lines are drawn; the rest of the buffer stays zero so
 * it displays as border colour once the bottom border opens.
 */
static void draw_checkerboard(uint8_t *buffer, const struct DemoConfig *cfg)
{
  uint16_t y, x;
  uint8_t *line = buffer + (uint32_t)cfg->visible_offset * cfg->line_bytes;
  uint16_t num_groups = cfg->line_bytes / 8;

/* Use fixed 32x32 pixel checkerboard (2 16-pixel groups per checker) */
#define CHECKER_SIZE 32
#define GROUPS_PER_CHECKER 2

  for (y = 0; y < cfg->visible_lines; ++y)
  {
    uint16_t *words;
    uint16_t y_checker = (y / CHECKER_SIZE) & 1;

    if (cfg->seam_row != 0 && y == cfg->seam_row)
    {
      line += cfg->line_bytes; /* Skip the buffer row the seam consumes */
    }
    words = (uint16_t *)line;

    for (x = 0; x < num_groups; ++x)
    {
      uint16_t x_checker = (x / GROUPS_PER_CHECKER) & 1;
      uint16_t pattern;

      pattern = (y_checker ^ x_checker) ? 0xFFFF : 0x0000;

      /* Write to bitplane 0 and 1 (words 0 and 1 of group) */
      words[x * 4 + 0] = pattern;
      words[x * 4 + 1] = pattern;
    }
    line += cfg->line_bytes;
  }

#undef CHECKER_SIZE
#undef GROUPS_PER_CHECKER
}

/* Gently pulse the light squares so it's visible the demo is alive */
static void update_palette(uint16_t frame)
{
  volatile uint16_t *hw = (volatile uint16_t *)0xffff8240;
  uint16_t t = (frame >> 2) & 7;

  if (t > 3)
  {
    t = 7 - t;
  }
  hw[3] = (uint16_t)(0x0444 + 0x0111 * t);
}

long demo_run(const struct DemoConfig *cfg)
{
  uint8_t *base = screen_base(scrbuf);
  uint16_t frame = 0;

  backbuf_flag = 0;
  scraddr1 = (uint32_t)(uintptr_t)base;
  scraddr2 = (uint32_t)(uintptr_t)(base + SCREEN_BYTES);

  disable_mouse();
  save_palette();
  set_palette(palette);

  draw_checkerboard((uint8_t *)scraddr1, cfg);
  draw_checkerboard((uint8_t *)scraddr2, cfg);

  if (cfg->setup)
  {
    cfg->setup();
  }

  while (1)
  {
    if (cfg->wait_vbl)
    {
      cfg->wait_vbl();
    }

    update_palette(frame++);

    if (*(volatile uint8_t *)0xfffffc02 == 0x39)
    {
      break;
    }
  }

  if (cfg->restore)
  {
    cfg->restore();
  }
  set_palette(save_pal);
  enable_mouse();

  return 0;
}
