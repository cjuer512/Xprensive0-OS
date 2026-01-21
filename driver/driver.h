#include "stdint.h"
extern void keyboard_init(unsigned char* idt);
extern void keyboard_handler(void);
extern void hdd_init_all(uint8_t* idt);
extern int hdd_read(uint32_t lba, uint8_t count, uint16_t* buffer);
extern int hdd_read_simple(uint32_t lba, uint16_t* buffer);
extern void vga_set_mode_13h(void);
extern void vga_set_palette_color(uint8_t index, uint8_t r, uint8_t g, uint8_t b);
extern void vga_put_pixel(int x, int y, uint8_t color);
extern void vga_clear_screen(uint8_t color);
extern void set_cursor_position(uint16_t pos);
extern uint16_t get_cursor_position(void);