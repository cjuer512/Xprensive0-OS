#include "../driver/stdint.h"
#include "../driver/stddef.h"
#include "../driver/driver.h"
#include "../api/api.h"
extern void keyboard_init(void);
__attribute__((noreturn)) void setup_keyboard_interrupt() {
    /* ====== 第一部分：在内存中搜索 keyboard_handler 签名 ====== */
    
    uint64_t handler_addr = (uint64_t)keyboard_handler;
    
    
    keyboard_init();
    
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
    __asm__ volatile("sti");
    __asm__ volatile("int $0x21");
    //int test = print("ABCDEFGHIJKLMNOPQRSTUVWXYZ");
    //if(test == 0){
    //    __asm__ volatile("nop");
    //}
    
    while(1){
        __asm__ volatile("nop");
    }
    // 4. 开中断（如果需要）
    
}