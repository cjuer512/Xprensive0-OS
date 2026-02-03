#include "stdint.h"
struct interrupt_frame
{
    uint64_t rip;    // 指令指针（返回地址）
    uint64_t cs;     // 代码段选择子
    uint64_t rflags; // 标志寄存器
    uint64_t rsp;    // 栈指针（如果发生特权级切换）
    uint64_t ss;     // 栈段选择子（如果发生特权级切换）
};
void keyboard_init(uint8_t *idt);
void keyboard_handler(struct interrupt_frame *frame);
void hdd_init_all(uint8_t* idt);
int hdd_read(uint32_t lba, uint8_t count, uint64_t* buffer);
int hdd_read_sectors(uint32_t lba, uint8_t sectors, uint16_t *buffer);
void vga_set_mode_13h(void);
void vga_set_palette_color(uint8_t index, uint8_t r, uint8_t g, uint8_t b);
void vga_put_pixel(int x, int y, uint8_t color);
void vga_clear_screen(uint8_t color);
void set_cursor_position(uint16_t pos);
uint16_t get_cursor_position(void);

#define HDD_H

#include "stdint.h"

void hdd_init_all(uint8_t *idt);
int hdd_read_simple(uint32_t lba, uint16_t *buffer,int byte_count,int start);
int hdd_read_sectors(uint32_t lba, uint8_t sectors, uint16_t *buffer);