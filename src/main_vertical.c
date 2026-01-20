#include <osbind.h>
#include <stdint.h>

#include "demo.h"

extern void overscan_ste_setup(void);
extern void overscan_ste_restore(void);

extern volatile uint16_t vblcnt;

static void wait_vbl(void)
{
  while (vblcnt == 0) {
  }
  vblcnt = 0;
}

static const struct DemoConfig demo_config = {
  160,
  268,
  0,
  76,
  0x000F,
  20,
  308,
  256,
  overscan_ste_setup,
  overscan_ste_restore,
  wait_vbl
};

static long demo(void)
{
  return demo_run(&demo_config);
}

int main(void)
{
  Supexec(demo);
  return 0;
}
