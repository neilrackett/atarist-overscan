#include <osbind.h>
#include <stdint.h>

#include "demo.h"

volatile uint32_t scraddr1;
volatile uint32_t scraddr2;
volatile uint8_t backbuf_flag;

static void *save_log;
static void *save_phys;

static void setup_screen(void)
{
  save_log = Logbase();
  save_phys = Physbase();
  Setscreen((void *)scraddr1, (void *)scraddr1, -1);
}

static void restore_screen(void)
{
  Setscreen(save_log, save_phys, -1);
}

static void wait_vbl(void)
{
  Vsync();
}

static const struct DemoConfig demo_config = {
  160,
  200,
  0,
  76,
  0x000F,
  20,
  308,
  188,
  setup_screen,
  restore_screen,
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
