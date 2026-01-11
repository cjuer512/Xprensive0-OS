#include "../driver/stdint.h"
#include "../driver/stddef.h"
#include "../driver/driver.h"
#include "../api/api.h"

__attribute__((noreturn)) void setup_keyboard_interrupt() {
    
    // 1. 在栈上创建IDT表（256个条目，4096字节）
    static uint8_t idt[4096] = {0};
    //2.加载驱动
    //2.1键盘驱动（一般来说其实应该是硬盘驱动，但是因为我先写的键盘驱动所以就先加载了）
    keyboard_init(idt);

    //2.2硬盘驱动
    hdd_init_all(idt);
    // 3. 加载IDT（lidt）
    struct {
        uint16_t limit;
        uint64_t base;
    } __attribute__((packed)) idtr;
    idtr.limit = sizeof(idt) - 1;
    idtr.base = (uint64_t)idt;
    
    __asm__ volatile("lidt %0" : : "m"(idtr));
    __asm__ volatile("sti");
    uint16_t big_buffer[256 * 4];  // 4个扇区
    hdd_read(100, 4, big_buffer);  // 从扇区100开始读4个扇区
    char* video1 = (char*)0xB8000;
    video1[0] = 'K';
    //__asm__ volatile("int $0x21");
    __asm__ volatile("int $0x2E");
    
    
    // 方法2：用中断方式读取（读取更多扇区）
    //uint16_t big_buffer[256 * 4];  // 4个扇区
    //hdd_read(100, 4, big_buffer);  // 从扇区100开始读4个扇区








    

    while(1){
        __asm__ volatile("nop");
    }
    // 4. 开中断（如果需要）
    
}