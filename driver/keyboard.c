/*void keyboard_init(void) {
    //测试函数：在屏幕上显示字符
    char* video = (char*)0xB8000;
    video[0] = 'K';
    video[1] = 0x0F;
}*/
#include "stdint.h" // 定义uint8_t, uint32_t等类型
#include "stddef.h" // 定义size_t, NULL等
#include "driverp.h"

uint16_t cursor_x = 0; // 当前列 (0-79)
uint16_t cursor_y = 0; // 当前行 (0-24)

/*static inline void outb(uint16_t port, uint8_t value);
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
}*/

// 3. 64位IDT设置函数
/*void set_idt_entry64(uint8_t index, uint64_t handler, uint16_t selector, uint8_t type_attr) {

    // 64位IDT条目结构不同！
}*/
#include "stdint.h"

// 获取光标位置（行，列）
uint16_t get_cursor_position(void)
{
    uint16_t pos = 0;
    uint8_t high, low; // 需要两个8位变量

    // 读取高字节
    __asm__ volatile("outb %b0, %w1" : : "a"((uint8_t)0x0E), "Nd"((uint16_t)0x3D4));
    __asm__ volatile("inb %w1, %b0" : "=a"(high) : "Nd"((uint16_t)0x3D5));

    // 读取低字节
    __asm__ volatile("outb %b0, %w1" : : "a"((uint8_t)0x0F), "Nd"((uint16_t)0x3D4));
    __asm__ volatile("inb %w1, %b0" : "=a"(low) : "Nd"((uint16_t)0x3D5));

    // 合并高字节和低字节
    pos = ((uint16_t)high << 8) | (uint16_t)low;

    return pos;
}
void set_cursor_position(uint16_t pos)
{
    // 设置高字节
    __asm__ volatile("outb %1, %0" : : "dN"((uint16_t)0x3D4), "a"((uint8_t)0x0E));
    __asm__ volatile("outb %1, %0" : : "dN"((uint16_t)0x3D5), "a"((uint8_t)((pos >> 8) & 0xFF)));

    // 设置低字节
    __asm__ volatile("outb %1, %0" : : "dN"((uint16_t)0x3D4), "a"((uint8_t)0x0F));
    __asm__ volatile("outb %1, %0" : : "dN"((uint16_t)0x3D5), "a"((uint8_t)(pos & 0xFF)));
}
char get_ascii_from_set2(uint8_t scancode)
{
    // 使用直接的 if-else 比较，避免编译器生成跳转表
    if (scancode == 0x15)
        return 'q';
    if (scancode == 0x1D)
        return 'w';
    if (scancode == 0x24)
        return 'e';
    if (scancode == 0x2D)
        return 'r';
    if (scancode == 0x2C)
        return 't';
    if (scancode == 0x35)
        return 'y';
    if (scancode == 0x3C)
        return 'u';
    if (scancode == 0x43)
        return 'i';
    if (scancode == 0x44)
        return 'o';
    if (scancode == 0x4D)
        return 'p';

    if (scancode == 0x1C)
        return 'a';
    if (scancode == 0x1B)
        return 's'; // S键
    if (scancode == 0x23)
        return 'd';
    if (scancode == 0x2B)
        return 'f';
    if (scancode == 0x34)
        return 'g';
    if (scancode == 0x33)
        return 'h';
    if (scancode == 0x3B)
        return 'j';
    if (scancode == 0x42)
        return 'k';
    if (scancode == 0x4B)
        return 'l';

    if (scancode == 0x1A)
        return 'z';
    if (scancode == 0x22)
        return 'x';
    if (scancode == 0x21)
        return 'c';
    if (scancode == 0x2A)
        return 'v';
    if (scancode == 0x32)
        return 'b';
    if (scancode == 0x31)
        return 'n';
    if (scancode == 0x3A)
        return 'm';

    if (scancode == 0x16)
        return '1';
    if (scancode == 0x1E)
        return '2';
    if (scancode == 0x26)
        return '3';
    if (scancode == 0x25)
        return '4';
    if (scancode == 0x2E)
        return '5';
    if (scancode == 0x36)
        return '6';
    if (scancode == 0x3D)
        return '7';
    if (scancode == 0x3E)
        return '8';
    if (scancode == 0x46)
        return '9';
    if (scancode == 0x45)
        return '0';

    if (scancode == 0x29)
        return ' ';
    if (scancode == 0x5A)
        return '\n';
    if (scancode == 0x66)
        return '\b';

    // 所有其他值，包括 0xF0, 0xE0，都返回 0
    return 0;
}
char get_ascii_from_set1(uint8_t scancode)
{
    // XT Set 1 通码 -> ASCII 映射 (小写字母和数字部分)
    // 使用 if-else 链，安全无跳转表
    if (scancode == 0x1E)
        return 'a'; // A
    if (scancode == 0x30)
        return 'b'; // B
    if (scancode == 0x2E)
        return 'c'; // C
    if (scancode == 0x20)
        return 'd'; // D
    if (scancode == 0x12)
        return 'e'; // E
    if (scancode == 0x21)
        return 'f'; // F (你按下F，现在会显示'f'了)
    if (scancode == 0x22)
        return 'g'; // G
    if (scancode == 0x23)
        return 'h'; // H
    if (scancode == 0x17)
        return 'i'; // I
    if (scancode == 0x24)
        return 'j'; // J
    if (scancode == 0x25)
        return 'k'; // K
    if (scancode == 0x26)
        return 'l'; // L
    if (scancode == 0x32)
        return 'm'; // M
    if (scancode == 0x31)
        return 'n'; // N
    if (scancode == 0x18)
        return 'o'; // O
    if (scancode == 0x19)
        return 'p'; // P
    if (scancode == 0x10)
        return 'q'; // Q
    if (scancode == 0x13)
        return 'r'; // R
    if (scancode == 0x1F)
        return 's'; // S (现在按S会有反应了)
    if (scancode == 0x14)
        return 't'; // T
    if (scancode == 0x16)
        return 'u'; // U
    if (scancode == 0x2F)
        return 'v'; // V
    if (scancode == 0x11)
        return 'w'; // W
    if (scancode == 0x2D)
        return 'x'; // X
    if (scancode == 0x15)
        return 'y'; // Y
    if (scancode == 0x2C)
        return 'z'; // Z

    // 数字行 (键盘上方)
    if (scancode == 0x02)
        return '1';
    if (scancode == 0x03)
        return '2';
    if (scancode == 0x04)
        return '3';
    if (scancode == 0x05)
        return '4';
    if (scancode == 0x06)
        return '5';
    if (scancode == 0x07)
        return '6';
    if (scancode == 0x08)
        return '7';
    if (scancode == 0x09)
        return '8';
    if (scancode == 0x0A)
        return '9';
    if (scancode == 0x0B)
        return '0';

    // 空格、回车等
    if (scancode == 0x39)
        return ' '; // 空格
    if (scancode == 0x1C)
        return '\n'; // 回车
    if (scancode == 0x0E)
        return '\b'; // 退格

    // 默认：不是已知通码，返回0
    return 0;
}
struct interrupt_frame
{
    uint64_t rip;    // 指令指针（返回地址）
    uint64_t cs;     // 代码段选择子
    uint64_t rflags; // 标志寄存器
    uint64_t rsp;    // 栈指针（如果发生特权级切换）
    uint64_t ss;     // 栈段选择子（如果发生特权级切换）
};
// 键盘中断处理函数（中断号0x21）
__attribute__((interrupt)) void keyboard_handler(struct interrupt_frame *frame)
{
    (void)frame; // 明确表示不使用，避免警告 {

    // 1. 读取键盘扫描码
    uint8_t scancode = inb(0x60);
    if (scancode == 0xF0)
    {

        // outb(0x20, 0x20);
        __asm__ volatile("nop" ::);
    }
    if (scancode == 0xE0)
    {

        // outb(0x20, 0x20);
        __asm__ volatile("nop" ::);
    }
    int scancode_int = (int)scancode;
    // AT Set 2 通码 -> ASCII 映射表 (小写/数字状态)
    // 索引=扫描码通码，值=对应的ASCII字符，0表示无映射或特殊键
    // 在全局区域定义这个“完美”映射表

    char ascii_code = get_ascii_from_set1(scancode);

    // char* video = (char*)0xB8000;
    // video[0] = set2_to_ascii[scancode];
    /*uint16_t pos = cursor_y * 80 + cursor_x;  // 计算位置
    char* video = (char*)0xB8000 + pos * 2;   // 定位到正确位置
    if(ascii_code!=0){
        video[0] = ascii_code;
        video[1] = 0x0F;
    }
    if(ascii_code!=0){
        cursor_x++;  // 列加1
    }
    if (cursor_x >= 80) {  // 如果到行尾
    cursor_x = 0;      // 回到行首
    cursor_y++;        // 换到下一行
    if (cursor_y >= 25) {  // 如果到底部
        cursor_y = 0;      // 回到顶部（简单处理）
    }
}*/
    char *buffer = (char *)0x8800;
    if (ascii_code != 0)
    {
        buffer[0] = buffer[1];
        buffer[1] = buffer[2];
        buffer[2] = buffer[3];
        buffer[3] = ascii_code;
        /*if (ascii_code = '\n')
        {
            uint16_t pos = 0;
            uint8_t high, low;

            // 读取高字节
            __asm__ volatile("outb %b0, %w1" : : "a"((uint8_t)0x0E), "Nd"((uint16_t)0x3D4));
            __asm__ volatile("inb %w1, %b0" : "=a"(high) : "Nd"((uint16_t)0x3D5));

            // 读取低字节
            __asm__ volatile("outb %b0, %w1" : : "a"((uint8_t)0x0F), "Nd"((uint16_t)0x3D4));
            __asm__ volatile("inb %w1, %b0" : "=a"(low) : "Nd"((uint16_t)0x3D5));

            // 合并高字节和低字节
            pos = ((uint16_t)high << 8) | (uint16_t)low;

            pos = (cursor_y + 1) * 80 + 0x0; // 计算位置
                                                      // 设置高字节
            __asm__ volatile("outb %1, %0" : : "dN"((uint16_t)0x3D4), "a"((uint8_t)0x0E));
            __asm__ volatile("outb %1, %0" : : "dN"((uint16_t)0x3D5), "a"((uint8_t)((pos >> 8) & 0xFF)));

            // 设置低字节
            __asm__ volatile("outb %1, %0" : : "dN"((uint16_t)0x3D4), "a"((uint8_t)0x0F));
            __asm__ volatile("outb %1, %0" : : "dN"((uint16_t)0x3D5), "a"((uint8_t)(pos & 0xFF)));
        }*/
    }
    else if (ascii_code = 0)
    {
        __asm__ volatile("nop");
    }

    // 3. 通知PIC中断处理结束
    outb(0x20, 0x20); // 向主PIC发送EOI
}
void keyboard_init(uint8_t *idt)
{
    uint64_t handler_addr = (uint64_t)keyboard_handler;
    /*该函数用来初始化键盘和其它驱动设置*/
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
    // 设置IDT条目，让中断0x21指向keyboard_handler
    // set_idt_entry64(0x21, (uint64_t)keyboard_handler, 0x08, 0x8E);
    // 最后，取消键盘的屏蔽（只开启IRQ1）
    outb(0x21, inb(0x21) & 0xFD); // 清除IRQ1的屏蔽位（位1）
    // 2. 只设置键盘中断条目（0x21）
    uint8_t *entry = &idt[0x21 * 16];

    // 填16字节...
    *(uint16_t *)(entry + 0) = handler_addr & 0xFFFF;
    *(uint16_t *)(entry + 2) = 0x0008;
    *(uint8_t *)(entry + 4) = 0x00;
    *(uint8_t *)(entry + 5) = 0x8E;
    *(uint16_t *)(entry + 6) = (handler_addr >> 16) & 0xFFFF;
    *(uint32_t *)(entry + 8) = handler_addr >> 32;
    *(uint32_t *)(entry + 12) = 0;
}