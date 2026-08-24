/*
 * Copyright (C) 2026 Neil Rackett
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <osbind.h>
#include <stdint.h>

#include "demo.h"

extern void overscan_bottom_setup(void);
extern void overscan_bottom_restore(void);

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
    0,   /* visible_offset */
    200, /* seam_row: the hidden line consumes buffer row 200 */
    overscan_bottom_setup,
    overscan_bottom_restore,
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
