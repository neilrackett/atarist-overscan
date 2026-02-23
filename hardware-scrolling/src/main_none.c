#include <osbind.h>
#include <stdint.h>

#include "demo.h"

volatile uint32_t scraddr1;
volatile uint32_t scraddr2;
volatile uint8_t backbuf_flag;
volatile uint16_t scroll_offset;
volatile uint8_t scroll_hpixel;

static void *save_log;
static void *save_phys;
static uint8_t save_hscroll;
static uint8_t save_loscroll;

static void setup_screen(void)
{
  save_log = Logbase();
  save_phys = Physbase();
  save_hscroll = *(volatile uint8_t *)0xFFFF820F;
  save_loscroll = *(volatile uint8_t *)0xFFFF820D;
  Setscreen((void *)scraddr1, (void *)scraddr1, -1);
}

static void restore_screen(void)
{
  /* Restore scroll registers */
  *(volatile uint8_t *)0xFFFF820F = save_hscroll;
  *(volatile uint8_t *)0xFFFF820D = save_loscroll;
  Setscreen(save_log, save_phys, -1);
}

static void wait_vbl(void)
{
  uint32_t display_addr;
  uint32_t temp;

  /* Wait for VBL first */
  Vsync();

  /* Calculate display address with scroll offset applied */
  /* Display scraddr1 which is the buffer we just finished drawing to */
  display_addr = scraddr1 + scroll_offset;

  /* Set screen address */
  *(volatile uint8_t *)0xFFFF8201 = (uint8_t)(display_addr >> 16);
  *(volatile uint8_t *)0xFFFF8203 = (uint8_t)(display_addr >> 8);
  *(volatile uint8_t *)0xFFFF820D = (uint8_t)(display_addr);

  /* Set horizontal pixel scroll (STE only, 0-15 pixels) */
  *(volatile uint8_t *)0xFFFF820F = scroll_hpixel;

  /* Swap buffer pointers - after this, scraddr1 points to the hidden buffer */
  temp = scraddr1;
  scraddr1 = scraddr2;
  scraddr2 = temp;
  backbuf_flag ^= 1;
}

static const struct DemoConfig demo_config = {
    160,    /* line_bytes */
    200,    /* visible_lines */
    0,      /* visible_offset */
    76,     /* right_border_word */
    0x000F, /* right_border_mask */
    20,     /* plane0_groups */
    308,    /* max_x */
    188,    /* max_y */
    32,     /* checker_size */
    1,      /* use_hw_scroll */
    setup_screen,
    restore_screen,
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
