#include "demo.h"

#include <stdint.h>

#define SCREEN_LINES 273
#define SCREEN_BYTES (61 * 1024)

extern volatile uint32_t scraddr1;
extern volatile uint32_t scraddr2;
extern volatile uint8_t backbuf_flag;

static uint8_t scrbuf[2 * SCREEN_BYTES + 256];
static uint16_t save_pal[16];

static const uint16_t palette[16] = {
  0x0777, /* 0: white background */
  0x0077, /* 1: turquoise border */
  0x0700, /* 2: red ball */
  0x0000, 0x0000, 0x0000, 0x0000,
  0x0000, 0x0000, 0x0000, 0x0000,
  0x0000, 0x0000, 0x0000, 0x0000
};

static const uint16_t ball_mask1[16] = {
  0xFF00, 0x7F80, 0x3FC0, 0x1FE0,
  0x0FF0, 0x07F8, 0x03FC, 0x01FE,
  0x00FF, 0x007F, 0x003F, 0x001F,
  0x000F, 0x0007, 0x0003, 0x0001
};

static const uint16_t ball_mask2[16] = {
  0x0000, 0x0000, 0x0000, 0x0000,
  0x0000, 0x0000, 0x0000, 0x0000,
  0x0000, 0x8000, 0xC000, 0xE000,
  0xF000, 0xF800, 0xFC00, 0xFE00
};

static uint16_t ball_x;
static uint16_t ball_y;
static int16_t ball_dx;
static int16_t ball_dy;
static uint16_t ball_prev1_x;
static uint16_t ball_prev1_y;
static uint16_t ball_prev2_x;
static uint16_t ball_prev2_y;

static void save_palette(void)
{
  volatile uint16_t *hw = (volatile uint16_t *)0xffff8240;
  uint16_t i;

  for (i = 0; i < 16; ++i) {
    save_pal[i] = hw[i];
  }
}

static void set_palette(const uint16_t *pal)
{
  volatile uint16_t *hw = (volatile uint16_t *)0xffff8240;
  uint16_t i;

  for (i = 0; i < 16; ++i) {
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

static void clear_buffer(uint8_t *buffer, uint16_t line_bytes)
{
  uint32_t *ptr = (uint32_t *)buffer;
  uint32_t count = ((uint32_t)line_bytes * SCREEN_LINES) / 4;

  while (count--) {
    *ptr++ = 0;
  }
}

static void fill_plane0_line(uint8_t *line, uint16_t groups)
{
  uint16_t i;
  uint32_t *ptr = (uint32_t *)line;

  for (i = 0; i < groups; ++i) {
    *ptr++ = 0xFFFF0000UL;
    *ptr++ = 0;
  }
}

static void draw_border(uint8_t *buffer, const struct DemoConfig *cfg)
{
  uint16_t i;
  uint8_t *line = buffer + cfg->visible_offset;

  for (i = 0; i < 4; ++i) {
    fill_plane0_line(line, cfg->plane0_groups);
    line += cfg->line_bytes;
  }

  line = buffer + (4 * cfg->line_bytes) + cfg->visible_offset;
  for (i = 0; i < (uint16_t)(cfg->visible_lines - 8); ++i) {
    uint16_t *words = (uint16_t *)line;
    words[0] |= 0xF000;
    words[cfg->right_border_word] |= cfg->right_border_mask;
    line += cfg->line_bytes;
  }

  line = buffer + ((cfg->visible_lines - 4) * cfg->line_bytes) + cfg->visible_offset;
  for (i = 0; i < 4; ++i) {
    fill_plane0_line(line, cfg->plane0_groups);
    line += cfg->line_bytes;
  }
}

static void blit_ball_word(uint8_t *buffer, const struct DemoConfig *cfg,
                           uint16_t x, uint16_t y, uint16_t word,
                           uint16_t x_offset, uint8_t op)
{
  uint16_t group = x >> 4;
  uint16_t offset = (uint16_t)(group * 8);
  uint8_t *line = buffer + (y * cfg->line_bytes) + cfg->visible_offset;
  uint16_t src[8];
  uint16_t row;
  short old_sr;

  for (row = 0; row < 8; ++row) {
    src[row] = word;
  }

  __asm__ volatile(
      "move.w %%sr, %0\n\t"
      "ori.w #0x0700, %%sr"
      : "=d"(old_sr)
      :
      : "cc");

  while (*(volatile uint8_t *)0xFF8A3C & 0x80)
    ;

  *(volatile short *)0xFF8A20 = 0; /* Src X Inc */
  *(volatile short *)0xFF8A22 = 2; /* Src Y Inc */
  *(volatile short *)0xFF8A2E = 0; /* Dst X Inc */
  *(volatile short *)0xFF8A30 = (short)cfg->line_bytes; /* Dst Y Inc */

  *(volatile short *)0xFF8A28 = 0xFFFF;
  *(volatile short *)0xFF8A2A = 0xFFFF;
  *(volatile short *)0xFF8A2C = 0xFFFF;

  *(volatile long *)0xFF8A24 = (long)src;
  *(volatile long *)0xFF8A32 = (long)(line + offset + x_offset);

  *(volatile short *)0xFF8A36 = 1;
  *(volatile short *)0xFF8A38 = 8;
  *(volatile char *)0xFF8A3A = 2; /* HOP: Source */
  *(volatile char *)0xFF8A3B = (char)op;
  *(volatile char *)0xFF8A3D = 0;

  *(volatile char *)0xFF8A3C = 0xC0;
  while (*(volatile char *)0xFF8A3C & 0x80)
    ;

  __asm__ volatile(
      "move.w %0, %%sr"
      :
      : "d"(old_sr)
      : "cc");
}

static void draw_ball_at(uint8_t *buffer, const struct DemoConfig *cfg, uint16_t x, uint16_t y)
{
  uint16_t within = x & 15;
  uint16_t mask1 = ball_mask1[within];
  uint16_t mask2 = ball_mask2[within];

  blit_ball_word(buffer, cfg, x, y, mask1, 2, 7);
  if (mask2 != 0) {
    blit_ball_word(buffer, cfg, x, y, mask2, 10, 7);
  }
}

static void erase_ball_at(uint8_t *buffer, const struct DemoConfig *cfg, uint16_t x, uint16_t y)
{
  uint16_t within = x & 15;
  uint16_t mask1 = (uint16_t)~ball_mask1[within];
  uint16_t mask2 = (uint16_t)~ball_mask2[within];

  blit_ball_word(buffer, cfg, x, y, mask1, 2, 1);
  if (mask2 != 0xFFFF) {
    blit_ball_word(buffer, cfg, x, y, mask2, 10, 1);
  }
}

static void update_ball(const struct DemoConfig *cfg)
{
  int16_t next_x = (int16_t)ball_x + ball_dx;
  if (next_x < 4) {
    next_x = 4;
    ball_dx = -ball_dx;
  } else if (next_x > (int16_t)cfg->max_x) {
    next_x = (int16_t)cfg->max_x;
    ball_dx = -ball_dx;
  }
  ball_x = (uint16_t)next_x;

  int16_t next_y = (int16_t)ball_y + ball_dy;
  if (next_y < 4) {
    next_y = 4;
    ball_dy = -ball_dy;
  } else if (next_y > (int16_t)cfg->max_y) {
    next_y = (int16_t)cfg->max_y;
    ball_dy = -ball_dy;
  }
  ball_y = (uint16_t)next_y;
}

static void init_scene(const struct DemoConfig *cfg)
{
  uint8_t *buf1;
  uint8_t *buf2;

  ball_x = 32;
  ball_y = 32;
  ball_dx = 4;
  ball_dy = 4;
  backbuf_flag = 0;

  buf1 = (uint8_t *)scraddr1;
  buf2 = (uint8_t *)scraddr2;
  clear_buffer(buf1, cfg->line_bytes);
  draw_border(buf1, cfg);
  draw_ball_at(buf1, cfg, ball_x, ball_y);
  clear_buffer(buf2, cfg->line_bytes);
  draw_border(buf2, cfg);
  draw_ball_at(buf2, cfg, ball_x, ball_y);

  ball_prev1_x = ball_x;
  ball_prev1_y = ball_y;
  ball_prev2_x = ball_x;
  ball_prev2_y = ball_y;
}

static void update_and_draw(const struct DemoConfig *cfg)
{
  uint8_t *buffer = (uint8_t *)scraddr1;

  update_ball(cfg);
  if (backbuf_flag) {
    erase_ball_at(buffer, cfg, ball_prev2_x, ball_prev2_y);
    draw_ball_at(buffer, cfg, ball_x, ball_y);
    ball_prev2_x = ball_x;
    ball_prev2_y = ball_y;
  } else {
    erase_ball_at(buffer, cfg, ball_prev1_x, ball_prev1_y);
    draw_ball_at(buffer, cfg, ball_x, ball_y);
    ball_prev1_x = ball_x;
    ball_prev1_y = ball_y;
  }
}

long demo_run(const struct DemoConfig *cfg)
{
  uint8_t *base = screen_base(scrbuf);

  scraddr1 = (uint32_t)(uintptr_t)base;
  scraddr2 = (uint32_t)(uintptr_t)(base + SCREEN_BYTES);

  disable_mouse();
  save_palette();
  set_palette(palette);

  init_scene(cfg);

  if (cfg->setup) {
    cfg->setup();
  }
  if (cfg->wait_vbl) {
    cfg->wait_vbl();
  }

  while (1) {
    if (cfg->wait_vbl) {
      cfg->wait_vbl();
    }

    update_and_draw(cfg);

    if (*(volatile uint8_t *)0xfffffc02 == 0x39) {
      break;
    }
  }

  if (cfg->restore) {
    cfg->restore();
  }
  set_palette(save_pal);
  enable_mouse();

  return 0;
}
