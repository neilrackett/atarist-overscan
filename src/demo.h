#ifndef DEMO_H
#define DEMO_H

#include <stdint.h>

struct DemoConfig {
  uint16_t line_bytes;
  uint16_t visible_lines;
  uint16_t visible_offset;
  uint16_t right_border_word;
  uint16_t right_border_mask;
  uint16_t plane0_groups;
  uint16_t max_x;
  uint16_t max_y;
  void (*setup)(void);
  void (*restore)(void);
  void (*wait_vbl)(void);
};

long demo_run(const struct DemoConfig *cfg);

#endif
