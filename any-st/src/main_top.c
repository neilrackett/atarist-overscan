/*
 * Copyright (C) 2026 Neil Rackett
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <osbind.h>
#include <stdint.h>

#include "demo.h"

extern void overscan_top_setup(void);
extern void overscan_top_restore(void);

extern volatile uint16_t vblcnt;

static void wait_vbl(void)
{
  while (vblcnt == 0)
  {
  }
  vblcnt = 0;
}

static const struct DemoConfig demo_config = {
    160, /* line_bytes */
    224, /* visible_lines */
    2,   /* visible_offset: lines 34-35 fetch but never display */
    0,   /* seam_row: none, the window is seamless */
    overscan_top_setup,
    overscan_top_restore,
    wait_vbl};

static long demo(void)
{
  return demo_run(&demo_config);
}

int main(void)
{
  Supexec(demo);
  return 0;
}
