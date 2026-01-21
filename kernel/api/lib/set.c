// ========== 设置IDT条目 ==========
static void set_idt_entry(uint8_t* idt, uint8_t vector, void* handler, uint16_t selector, uint8_t type_attr) {
    uint64_t handler_addr = (uint64_t)handler;
    uint8_t* entry = idt + (vector * 16);  // 每个条目16字节
    
    // 设置低32位（偏移低16位 + 段选择子 + 属性）
    *(uint16_t*)(entry + 0) = handler_addr & 0xFFFF;          // 偏移低16位
    *(uint16_t*)(entry + 2) = selector;                       // 段选择子
    *(uint8_t*)(entry + 4) = 0x00;                            // 保留
    *(uint8_t*)(entry + 5) = type_attr;                       // 类型属性
    
    // 设置中间32位（偏移中16位 + 偏移高32位）
    *(uint16_t*)(entry + 6) = (handler_addr >> 16) & 0xFFFF;  // 偏移中16位
    *(uint32_t*)(entry + 8) = handler_addr >> 32;             // 偏移高32位
    
    // 最后4字节为0
    *(uint32_t*)(entry + 12) = 0x00000000;
}

// ========== 字符打印中断处理函数 ==========
__attribute__((interrupt))
static void print_char_handler(void* frame) {
    // 获取字符和颜色（通过寄存器传递）
    uint32_t character, color, x, y;
    
    // 从frame结构获取参数（假设frame结构包含寄存器）
    // 这里简化处理，直接从寄存器读取
    __asm__ volatile(
        "mov %%eax, %0\n"
        "mov %%ebx, %1\n"
        "mov %%ecx, %2\n"
        "mov %%edx, %3"
        : "=r"(character), "=r"(color), "=r"(x), "=r"(y)
    );
    
    // 检查是否为文本模式（0xB8000）
    if (*(uint16_t*)0x410 & 0x30) {  // 检查显示模式
        // 文本模式：80x25
        uint16_t* video = (uint16_t*)0xB8000;
        uint16_t pos = y * 80 + x;
        if (pos < 80 * 25) {
            video[pos] = (color << 8) | (character & 0xFF);
        }
    } else {
        // 图形模式13h：320x200
        uint8_t* video = (uint8_t*)0xA0000;
        // 这里可以添加图形模式的字符绘制
        // 简化的实现：在指定位置画一个像素
        uint32_t pos = y * 320 + x;
        if (pos < 320 * 200) {
            video[pos] = (uint8_t)character;
        }
    }
    
    // 发送EOI（如果是硬件中断）
    // 对于软件中断（int指令），不需要EOI
    // outb(0x20, 0x20);  // 如果需要的话
}

// ========== 字符打印中断初始化函数 ==========
/**
 * 初始化字符打印中断
 * @param idt IDT表地址
 * @param vector 中断向量号（建议使用0x60-0x6F之间的用户定义中断）
 */
void print_char_init(uint8_t* idt, uint8_t vector) {
    // 设置中断向量
    // 0xEE = 0b11101110: P=1, DPL=3(用户可调用), 类型=中断门
    set_idt_entry(idt, vector, (void*)print_char_handler, 0x08, 0xEE);
    
    // 可选：在控制台输出调试信息
    // uint16_t* video = (uint16_t*)0xB8000;
    // video[0] = (0x0F << 8) | 'P';  // 显示'P'表示print中断已加载
    // video[1] = (0x0F << 8) | 'C';
}

// ========== 用户可调用的打印函数（在用户程序中使用） ==========
/**
 * 用户程序调用的打印字符函数
 * @param ch 要打印的字符
 * @param color 颜色属性（文本模式）
 * @param x X坐标
 * @param y Y坐标
 */
void print_char(char ch, uint8_t color, uint8_t x, uint8_t y) {
    __asm__ volatile(
        "mov $0x60, %%al\n"      // 假设使用0x60作为print中断号
        "int $0x60\n"
        : 
        : "a"(ch), "b"(color), "c"(x), "d"(y)
        : "memory"
    );
}

// ========== 你的setup函数中调用 ==========
__attribute__((noreturn)) void setup_keyboard_interrupt() {
    __asm__ volatile("cli");
    
    // 1. 切换VGA模式
    vga_set_mode_13h();
    
    // 2. 初始化调色板
    outb(0x3C8, 0);
    for(int i=0; i<256; i++) {
        outb(0x3C9, i >> 2);
        outb(0x3C9, i >> 2);
        outb(0x3C9, i >> 2);
    }
    
    // 3. 加载各种驱动和中断
    keyboard_init(idt);
    hdd_init_all(idt);
    
    // 4. 加载你的字符打印中断（使用0x60作为向量）
    print_char_init(idt, 0x60);
    
    // 5. 加载IDT
    struct {
        uint16_t limit;
        uint64_t base;
    } __attribute__((packed)) idtr;
    idtr.limit = sizeof(idt) - 1;
    idtr.base = (uint64_t)idt;
    __asm__ volatile("lidt %0" : : "m"(idtr));
    
    __asm__ volatile("sti");
    
    // 6. 测试：使用中断打印字符
    // 方法1：直接调用中断
    __asm__ volatile(
        "mov $0x41, %%eax\n"      // 字符'A'
        "mov $0x0F, %%ebx\n"      // 颜色：白底黑字
        "mov $10, %%ecx\n"        // x=10
        "mov $5, %%edx\n"         // y=5
        "int $0x60"
        : : : "eax", "ebx", "ecx", "edx"
    );
    
    // 方法2：使用封装函数
    print_char('B', 0x0F, 20, 5);
    
    while(1) {
        __asm__ volatile("hlt");
    }
}