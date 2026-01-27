#include "../driver/stdint.h"
#include "../driver/stddef.h"
#include "../driver/driver.h"
#include "../driver/driverp.h"
#include "../kernel/kernel.h"
__attribute__((aligned(4096))) static uint8_t idt[4096] = {0};  // IDT表（4KB，对齐到4KB）
__attribute__((aligned(512))) static uint64_t big_buffer[256 * 4] = {0};  // 硬盘缓冲区（2KB，对齐到512）
extern void kernel_init();
//魔术，启动时用
const char boot_magic[] __attribute__((section(".boot_magic"))) = "cjuer";
__attribute__((noreturn)) void loader64_main() {
    __asm__ volatile("cli");
    // 1. 先切换VGA 13h模式（必须放最前面！）
    //vga_set_mode_13h();
    
    // 2. 初始化调色板（模式切换后再做，且修正数值范围）
    /*outb(0x3C8, 0); // 从索引0开始设置
    for(int i=0; i<256; i++) {
        outb(0x3C9, i >> 2);   // 红色分量（0~63）
        outb(0x3C9, i >> 2);   // 绿色分量
        outb(0x3C9, i >> 2);   // 蓝色分量
    }*/
    
    // 3. 往显存写入渐变值（最后写）
    // 替换你当前的清屏代码
    //vga_set_palette_color(1, 0x00, 0x00, 0xFF); // 索引1=纯蓝
    //vga_clear_screen(1); // 清屏为蓝色
    outb(0x20, 0x11); // 向主PIC发送ICW1
    // 创造延迟
    for (int i = 0; i < 100; i++)
    {
        __asm__ volatile("nop"); // 空操作指令
    }
    outb(0xA0, 0x11); // 向从PIC发送ICW1
    // 设置主PIC：IRQ0-IRQ7映射到0x20-0x27
    outb(0x21, 0x20); // 向主PIC数据端口发送ICW2
    outb(0xA1, 0x28); // 向从PIC数据端口发送ICW2
    outb(0x21, 0x04); // ICW3: 主PIC IRQ2连接从PIC
    outb(0xA1, 0x02); // ICW3: 从PIC连接到主PIC IRQ2
    outb(0x21, 0x01); // ICW4: 8086模式
    outb(0xA1, 0x01);
    outb(0x21, 0xFD); // OCW1: 屏蔽所有中断，只开启键盘(IRQ1)

    outb(0xA1, 0xFF); // OCW1: 屏蔽所有从PIC中断
    //2.加载驱动
    //2.1键盘驱动（一般来说其实应该是硬盘驱动，但是因为我先写的键盘驱动所以就先加载了）
    keyboard_init(idt);

    //2.2硬盘驱动
    //切记！！！这个硬盘驱动有着配置pic的功能，删了会崩溃
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
    //hdd_read(100, 4, big_buffer);  // 从扇区100开始读4个扇区
    //char* video1 = (char*)0xB8000;
    //video1[0] = 'K';
    //__asm__ volatile("int $0x21");
    //__asm__ volatile("int $0x2E");
    
    // 4. 开中断（如果需要）
    kernel_init();
    while(1){
        __asm__ volatile("nop");
    }
}