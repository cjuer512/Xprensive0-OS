#include "stddef.h"
#include "stdint.h"

void outb(uint16_t port, uint8_t value) {
    // 关键修正：使用“outb %%al, %%dx”的显式模板，并添加"memory"破坏描述
    __asm__ volatile ("outb %b0, %w1"
                  :
                  : "a" (value), "Nd" (port)
                  : "memory"); // 告诉编译器内存可能被更改
}

uint8_t inb(uint16_t port) {
    uint8_t ret;
    // 关键修正：使用“inb %%dx, %%al”的显式模板
    __asm__ volatile ("inb %w1, %b0"
                  : "=a" (ret)
                  : "Nd" (port)
                  : "memory");
    return ret;
}