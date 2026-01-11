#include "../driver/stdint.h"
#include "../driver/stddef.h"
#include "../driver/driver.h"
#include "../api/api.h"
__attribute__((aligned(4096))) static uint8_t idt[4096] = {0};  // IDT表（4KB，对齐到4KB）
__attribute__((aligned(512))) static uint16_t big_buffer[256 * 4] = {0};  // 硬盘缓冲区（2KB，对齐到512）
__attribute__((noreturn)) void setup_keyboard_interrupt() {
    
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
    hdd_read(100, 4, big_buffer);  // 从扇区100开始读4个扇区
    char* video1 = (char*)0xB8000;
    video1[0] = 'K';
    //__asm__ volatile("int $0x21");
    __asm__ volatile("int $0x2E");

    // 1. 设置模式
    vga_set_mode_13h();
    
    volatile uint8_t* test_fb = (uint8_t*)0xA0000;
    for (int i = 0; i < 320*200; i++) {
        test_fb[i] = i % 256; // 写入渐变值
    }

    // 2. 设置几个调色板颜色
    /*vga_set_palette_color(0, 0, 0, 0);        // 颜色0 = 黑色
    vga_set_palette_color(1, 255, 0, 0);      // 颜色1 = 红色
    vga_set_palette_color(2, 0, 255, 0);      // 颜色2 = 绿色
    vga_set_palette_color(3, 0, 0, 255);      // 颜色3 = 蓝色
    
    // 3. 清屏为黑色
    vga_clear_screen(0);
    
    // 4. 画一个红色像素
    vga_put_pixel(100, 100, 1);  // 使用颜色1（红色）
    
    // 5. 画一条绿色线
    for(int x = 50; x < 150; x++) {
        vga_put_pixel(x, 50, 2);  // 绿色
    }*/








    

    while(1){
        __asm__ volatile("nop");
    }
    // 4. 开中断（如果需要）
    
}