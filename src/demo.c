#include "demo.h"

#include <stdint.h>

/* Need extra lines for vertical scrolling - visible lines + max scroll offset */
/* Max scroll is 64 lines (2 x 32-pixel checker), visible is up to 268 */
#define SCREEN_LINES 340
#define SCREEN_BYTES (80 * 1024)

extern volatile uint32_t scraddr1;
extern volatile uint32_t scraddr2;
extern volatile uint8_t backbuf_flag;

/* Hardware scroll values - updated by demo, applied during VBL */
volatile uint16_t scroll_offset; /* Line offset for vertical scroll */
volatile uint8_t scroll_hpixel;  /* Horizontal pixel scroll (0-15) */

static uint8_t scrbuf[2 * SCREEN_BYTES + 256];
static uint16_t save_pal[16];

/* Parallax scroll position (16.8 fixed point for smoother movement) */
static uint32_t parallax_y;

static const uint16_t palette[16] = {
    0x0777, /* 0: white (checkerboard light) */
    0x0000, /* 1: black (checkerboard dark) */
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
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
 * Color 0 (planes 0=0, 1=0) = white/light
 * Color 3 (planes 0=1, 1=1) = black/dark
 *
 * The checkerboard is drawn across the entire screen buffer to allow
 * for hardware scrolling. Using 32x32 pixel squares (2 groups x 32 lines).
 */
static void draw_checkerboard(uint8_t *buffer, const struct DemoConfig *cfg)
{
  uint16_t y, x;
  uint8_t *line = buffer;
  uint16_t num_groups = cfg->line_bytes / 8;

/* Use fixed 32x32 pixel checkerboard (2 16-pixel groups per checker) */
#define CHECKER_SIZE 32
#define GROUPS_PER_CHECKER 2

  for (y = 0; y < SCREEN_LINES; ++y)
  {
    uint16_t *words = (uint16_t *)line;
    /* y_checker: which vertical checker band (0 or 1, alternating every 32 lines) */
    uint16_t y_checker = (y / CHECKER_SIZE) & 1;

    for (x = 0; x < num_groups; ++x)
    {
      /* x_checker: which horizontal checker band (0 or 1, alternating every 2 groups) */
      uint16_t x_checker = (x / GROUPS_PER_CHECKER) & 1;
      uint16_t pattern;

      /* XOR to create checkerboard - dark squares when y_checker ^ x_checker */
      pattern = (y_checker ^ x_checker) ? 0xFFFF : 0x0000;

      /* Write to bitplane 0 and 1 (words 0 and 1 of group) */
      /* Plane 0 at word offset 0, Plane 1 at word offset 1 */
      words[x * 4 + 0] = pattern;
      words[x * 4 + 1] = pattern;
      /* Bitplanes 2 and 3 (words 2 and 3) stay 0/unchanged */
    }
    line += cfg->line_bytes;
  }

#undef CHECKER_SIZE
#undef GROUPS_PER_CHECKER
}

/*
 * Update the parallax scroll position.
 * The background scrolls continuously for a parallax effect.
 * Uses STE hardware scroll registers for flicker-free scrolling.
 */
static void update_parallax(const struct DemoConfig *cfg)
{
  if (!cfg->use_hw_scroll)
  {
    return;
  }

  /* Update parallax position - continuous scrolling */
  parallax_y += 256; /* Move 1 pixel per frame */

  /* Wrap parallax position to stay within checker pattern period */
  /* For a 32-pixel checker, we need 64 pixel wrap (2 squares) */
  uint16_t wrap_y = (cfg->checker_size ? cfg->checker_size : 32) * 2;

  if ((parallax_y >> 8) >= wrap_y)
  {
    parallax_y -= (uint32_t)wrap_y << 8;
  }

  /* Calculate hardware scroll values */
  uint16_t py = (uint16_t)(parallax_y >> 8);

  /* Horizontal pixel scroll disabled for simplicity */
  scroll_hpixel = 0;

  /* Vertical scroll is done by adjusting screen address offset */
  /* Each line is line_bytes, so multiply vertical pixel offset by line_bytes */
  scroll_offset = (uint16_t)(py * cfg->line_bytes);
}

static void init_scene(const struct DemoConfig *cfg)
{
  uint8_t *buf1;
  uint8_t *buf2;

  backbuf_flag = 0;

  /* Initialize parallax position */
  parallax_y = 0;
  scroll_offset = 0;
  scroll_hpixel = 0;

  buf1 = (uint8_t *)scraddr1;
  buf2 = (uint8_t *)scraddr2;

  /* Draw checkerboard background on both buffers */
  draw_checkerboard(buf1, cfg);
  draw_checkerboard(buf2, cfg);
}

static void update_and_draw(const struct DemoConfig *cfg)
{
  update_parallax(cfg);
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

  if (cfg->setup)
  {
    cfg->setup();
  }
  if (cfg->wait_vbl)
  {
    cfg->wait_vbl();
  }

  while (1)
  {
    if (cfg->wait_vbl)
    {
      cfg->wait_vbl();
    }

    update_and_draw(cfg);

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
