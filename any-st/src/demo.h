/*
 * Copyright (C) 2026 Neil Rackett
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef DEMO_H
#define DEMO_H

#include <stdint.h>

struct DemoConfig
{
  uint16_t line_bytes;
  uint16_t visible_lines;  /* Lines the demo draws (the game window) */
  uint16_t visible_offset; /* Buffer rows consumed by hidden lines before
                              the first visible one */
  uint16_t seam_row;       /* Content row where the hardware consumes one
                              buffer row invisibly (0 = none): content from
                              this row on is drawn one buffer row lower so
                              nothing is lost under the seam */
  void (*setup)(void);
  void (*restore)(void);
  void (*wait_vbl)(void);
};

long demo_run(const struct DemoConfig *cfg);

#endif
