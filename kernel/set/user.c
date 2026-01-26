// kernel/user.c
#include "stdint.h"
#include "../api/api.h"

// ------------ 全局常量定义 ------------
#define USER_STACK_SIZE 4096
#define VIDEO_MEM_ADDR 0xB8000
#define PAGE_PRESENT (1 << 0)
#define PAGE_WRITABLE (1 << 1)
#define PAGE_USER (1 << 2)
#define PAGE_PS (1 << 7)
#define PAGE_FRAME_MASK 0x000FFFFF000ULL

// ------------ 全局变量 ------------
static uint8_t user_stack[USER_STACK_SIZE] __attribute__((aligned(16)));

// ------------ 辅助函数：打印单个字符（基础） ------------
static void vga_putc(int x, int y, char c, uint8_t attr)
{
    char *video = (char *)VIDEO_MEM_ADDR;
    int offset = (y * 80 + x) * 2;
    video[offset] = c;
    video[offset + 1] = attr;
}

// ------------ 辅助函数：打印字符串（解决多字符问题） ------------
static void vga_puts(int x, int y, const char *str, uint8_t attr)
{
    while (*str)
    {
        vga_putc(x++, y, *str++, attr);
        if (x >= 80)
        {
            x = 0;
            y++;
        }
    }
}

// ------------ 核心函数：创建用户页表 ------------
void setup_user_page_table(void)
{
    vga_puts(0, 1, "CREATE PT", 0x0F);

    uint64_t *user_pml4 = (uint64_t *)0xC000;
    uint64_t *user_pdpt = (uint64_t *)0xD000;
    uint64_t *user_pdt = (uint64_t *)0xE000;

    for (int i = 0; i < 512; i++)
    {
        user_pml4[i] = 0;
        user_pdpt[i] = 0;
        user_pdt[i] = 0;
    }

    user_pml4[0] = ((uint64_t)user_pdpt) | PAGE_PRESENT | PAGE_WRITABLE | PAGE_USER;
    user_pdpt[0] = ((uint64_t)user_pdt) | PAGE_PRESENT | PAGE_WRITABLE | PAGE_USER;
    user_pdt[0] = 0x000000 | PAGE_PRESENT | PAGE_WRITABLE | PAGE_USER | PAGE_PS;
    user_pdt[28] = (0xB8000 & PAGE_FRAME_MASK) | PAGE_PRESENT | PAGE_WRITABLE | PAGE_USER | PAGE_PS;

    vga_puts(9, 1, " OK", 0x0F);
    __asm__ volatile("movq %0, %%cr3" : : "r"((uint64_t)user_pml4) : "memory");
    vga_puts(12, 1, " CR3 OK", 0x0F);
}

// ------------ 用户态代码 ------------
static void user_code(void)
{
    // 用户态写显存：标记进入用户态
    char *video = (char *)VIDEO_MEM_ADDR;
    int offset = 2 * 80 * 2; // 第三行开始打印
    const char *msg = "USER MODE RUNNING";
    while (*msg)
    {
        video[offset] = *msg++;
        video[offset + 1] = 0x0F; // 白字黑底
        offset += 2;
    }

    // ========== 核心：用户态触发syscall ==========
    uint64_t syscall_num = 1; // 对应内核定义的SYS_TEST=1
    uint64_t ret_value;       // 保存syscall返回值

    __asm__ volatile(
        // x86_64 syscall调用规范：
        // rax = 系统调用号
        // rdi/rsi/rdx/r10/r8/r9 = 参数1-6
        // 执行syscall指令触发内核调用
        "movq %0, %%rax\n\t"                 // rax = 系统调用号(1)
        "syscall\n\t"                        // 触发syscall，跳转到内核syscall_entry
        "movq %%rax, %1\n\t"                 // 保存返回值到ret_value
        : "=r"(syscall_num), "=r"(ret_value) // 输出（占位+返回值）
        : "0"(syscall_num)                   // 输入：系统调用号
        : "rcx", "r11", "memory"             // 破坏的寄存器（syscall会覆盖rcx/r11）
    );

    // 用户态打印syscall返回值（验证调用成功）
    offset = 3 * 80 * 2; // 第四行打印返回值
    const char *ret_msg = "syscall ret: 0x12345678";
    if (ret_value != 0x12345678)
    {
        ret_msg = "syscall ret: error";
    }
    while (*ret_msg)
    {
        video[offset] = *ret_msg++;
        video[offset + 1] = 0x0F;
        offset += 2;
    }

    // 用户态死循环
    while (1)
    {
        __asm__ volatile("nop");
    }
}

// ------------ 核心函数：切换到用户态（彻底修复汇编错误） ------------
__attribute__((noreturn)) void switch_to_user(
    //uint64_t *kernel_stack
)
{
    vga_puts(0, 0, "START USER", 0x0F);
    //__asm__ volatile("cli");
    vga_puts(10, 0, " CLI OK", 0x0F);
    setup_user_page_table();

    //uint64_t user_rsp = (uint64_t)&user_stack[USER_STACK_SIZE];
    //user_rsp &= ~0xF;
    uint64_t user_rsp = (uint64_t)0;
    // 关键：用数字直接定义段选择子，避免寄存器类型推导错误
    const uint64_t USER_DATA_SEL = 0x23;
    const uint64_t USER_CODE_SEL = 0x1B;

    // 彻底重写汇编块：纯64位操作，无16位寄存器混用
    __asm__ __volatile__(
    // 步骤1：设置用户态段寄存器
    "movq %0, %%rax\n\t"
    "movw %%ax, %%ds\n\t"
    "movw %%ax, %%es\n\t"
    "movw %%ax, %%fs\n\t"
    "movw %%ax, %%gs\n\t"

    // 步骤2：构建iretq栈帧
    "pushq %%rax\n\t"         // SS
    "pushq %1\n\t"            // RSP
    "pushfq\n\t"              // RFLAGS
    "orq $0x200, (%%rsp)\n\t" // 开启IF位
    "pushq %2\n\t"            // CS
    "pushq %3\n\t"            // RIP

    // 步骤3：切换到用户态
    "iretq\n\t"
    "ud2\n\t"

    : // 无输出
    : "r"(USER_DATA_SEL),     // %0
      "r"(user_rsp),          // %1  
      "r"(USER_CODE_SEL),     // %2
      "r"((uint64_t)user_code) // %3
    : "rax", "memory", "cc"
);

    __builtin_unreachable();
}