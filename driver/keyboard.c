/*void keyboard_init(void) {
    //测试函数：在屏幕上显示字符
    char* video = (char*)0xB8000;
    video[0] = 'K';
    video[1] = 0x0F;
}*/
#include "stdint.h"    // 定义uint8_t, uint32_t等类型
#include "stddef.h"    // 定义size_t, NULL等
//--------------------我也不知道为什么要写这个，我折磨半天了，问deepseek告诉我得写这些-------
static inline void outb(uint16_t port, uint8_t value);
static inline uint8_t inb(uint16_t port);
void set_idt_entry64(uint8_t index, uint64_t handler, uint16_t selector, uint8_t type_attr);

// 2. 实现端口I/O（64位同样适用）
static inline void outb(uint16_t port, uint8_t value) {
    asm volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    asm volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

// 3. 64位IDT设置函数
/*void set_idt_entry64(uint8_t index, uint64_t handler, uint16_t selector, uint8_t type_attr) {
    // 你需要实现这个函数，或者修改现有实现
    // 64位IDT条目结构不同！
}*/
//-------------------下面是我写的了-----------------------------



struct interrupt_frame {
    uint64_t rip;     // 指令指针（返回地址）
    uint64_t cs;      // 代码段选择子
    uint64_t rflags;  // 标志寄存器
    uint64_t rsp;     // 栈指针（如果发生特权级切换）
    uint64_t ss;      // 栈段选择子（如果发生特权级切换）
};
// 键盘中断处理函数（中断号0x21）
__attribute__((interrupt)) 
void keyboard_handler(struct interrupt_frame* frame) {
    (void)frame; // 明确表示不使用，避免警告 {
    
    // 1. 读取键盘扫描码
    uint8_t scancode = inb(0x60);
    
    char* video = (char*)0xB8000;
    video[2] = scancode;
    video[3] = 0x0E;
    // 3. 通知PIC中断处理结束
    outb(0x20, 0x20);  // 向主PIC发送EOI
}
void keyboard_init(void){
    /*该函数用来初始化键盘和其它驱动设置*/
    outb(0x20, 0x11);  // 向主PIC发送ICW1
    //创造延迟
    for(int i = 0; i < 100; i++){
        __asm__ volatile("nop"); // 空操作指令
    }
    outb(0xA0, 0x11);  // 向从PIC发送ICW1
    // 设置主PIC：IRQ0-IRQ7映射到0x20-0x27
    outb(0x21, 0x20);  // 向主PIC数据端口发送ICW2
    outb(0xA1, 0x28);  // 向从PIC数据端口发送ICW2
    outb(0x21, 0x04);  // ICW3: 主PIC IRQ2连接从PIC
    outb(0xA1, 0x02);  // ICW3: 从PIC连接到主PIC IRQ2
    outb(0x21, 0x01);  // ICW4: 8086模式
    outb(0xA1, 0x01);
    outb(0x21, 0xFD);  // OCW1: 屏蔽所有中断，只开启键盘(IRQ1)
    outb(0xA1, 0xFF); // OCW1: 屏蔽所有从PIC中断
    // 设置IDT条目，让中断0x21指向keyboard_handler
    //set_idt_entry64(0x21, (uint64_t)keyboard_handler, 0x08, 0x8E);
    // 最后，取消键盘的屏蔽（只开启IRQ1）
    outb(0x21, inb(0x21) & 0xFD);  // 清除IRQ1的屏蔽位（位1）
    
}