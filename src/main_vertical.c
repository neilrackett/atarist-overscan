#include <osbind.h>
#include <stdint.h>

#include "demo.h"

extern void overscan_ste_setup(void);
extern void overscan_ste_restore(void);

extern volatile uint16_t vblcnt;

static void wait_vbl(void)
{
  while (vblcnt == 0)
  {
  }
  vblcnt = 0;
}

static const struct DemoConfig demo_config = {
    160,    /* line_bytes */
    268,    /* visible_lines */
    0,      /* visible_offset */
    76,     /* right_border_word */
    0x000F, /* right_border_mask */
    20,     /* plane0_groups */
    308,    /* max_x */
    256,    /* max_y */
    32,     /* checker_size */
    1,      /* use_hw_scroll */
    overscan_ste_setup,
    overscan_ste_restore,
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
