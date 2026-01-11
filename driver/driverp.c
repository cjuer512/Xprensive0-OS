#include "stddef.h"
#include "stdint.h"
void outb(uint16_t port, uint8_t value) {
    asm volatile ("outb %0, %1" : : "a" (value), "Nd" (port));
}

// 内联函数，用于从端口读取一个字节的数据
uint8_t inb(uint16_t port) {
    uint8_t ret;
    asm volatile ("inb %1, %0" : "=a" (ret) : "Nd" (port));
    return ret;
}