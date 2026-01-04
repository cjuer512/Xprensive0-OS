#include "driver/stdint.h"
#include "driver/stddef.h"
void setup_keyboard_interrupt(uint64_t handler_addr) {
    // 1. 在栈上创建IDT表（256个条目，4096字节）
    static uint8_t idt[4096] = {0};
    
    // 2. 只设置键盘中断条目（0x21）
    uint8_t* entry = &idt[0x21 * 16];
    // 填16字节...
    *(uint16_t*)(entry + 0) = handler_addr & 0xFFFF;
    *(uint16_t*)(entry + 2) = 0x0008;
    *(uint8_t*)(entry + 4) = 0x00;
    *(uint8_t*)(entry + 5) = 0x8E;
    *(uint16_t*)(entry + 6) = (handler_addr >> 16) & 0xFFFF;
    *(uint32_t*)(entry + 8) = handler_addr >> 32;
    *(uint32_t*)(entry + 12) = 0;
    
    // 3. 加载IDT（lidt）
    struct {
        uint16_t limit;
        uint64_t base;
    } __attribute__((packed)) idtr;
    idtr.limit = sizeof(idt) - 1;
    idtr.base = (uint64_t)idt;
    __asm__ volatile("lidt %0" : : "m"(idtr));
    while (1)
    {
        __asm__ volatile("nop");
    }
    
    
    // 4. 开中断（如果需要）
    // __asm__ volatile("sti");
}