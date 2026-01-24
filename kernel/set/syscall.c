// src/kernel/syscall.c
#include "stdint.h"
#include "syscall.h"
#include "../api/api.h"

// 全局变量
static struct {
    uint64_t rax, rdi, rsi, rdx, r10, r8, r9, rcx, r11;
} syscall_regs;

__attribute__((naked))
void syscall_entry(void) {
    __asm__ volatile (
        // 保存rcx和r11（sysret需要）
        "pushq %rcx\n\t"
        "pushq %r11\n\t"
        
        // 保存其他寄存器到全局变量
        "movq %rax, syscall_regs(%rip)\n\t"
        "movq %rdi, syscall_regs+8(%rip)\n\t"
        "movq %rsi, syscall_regs+16(%rip)\n\t"
        "movq %rdx, syscall_regs+24(%rip)\n\t"
        "movq %r10, syscall_regs+32(%rip)\n\t"
        "movq %r8, syscall_regs+40(%rip)\n\t"
        "movq %r9, syscall_regs+48(%rip)\n\t"
        
        "movq %rcx, syscall_regs+56(%rip)\n\t"  // 保存rcx
        "movq %r11, syscall_regs+64(%rip)\n\t"  // 保存r11
        // 调用C处理函数
        "pushq %rbp\n\t"
        "movq %rsp, %rbp\n\t"
        "call syscall_handler\n\t"
        "popq %rbp\n\t"
        
        // 从全局变量恢复rcx和r11
        "movq syscall_regs+56(%rip), %rcx\n\t"
        "movq syscall_regs+64(%rip), %r11\n\t"
        
        // 返回用户态
        "sysretq\n\t"
    );
}

// C处理函数
void syscall_handler(void) {
    uint64_t num = syscall_regs.rax;
    
    if (num == 1) {
        // 只用 print_line
        print_line("wuhuhu\n");
        syscall_regs.rax = 0x12345678;  // 返回测试值
    } else {
        syscall_regs.rax = 0xFFFFFFFFFFFFFFFF;  // 错误
    }
}

// MSR操作
static inline void cpu_write_msr(uint32_t msr, uint64_t value) {
    __asm__ volatile("wrmsr" : : "c"(msr), "a"((uint32_t)(value & 0xFFFFFFFF)), 
                      "d"((uint32_t)(value >> 32)));
}

static inline uint64_t cpu_read_msr(uint32_t msr) {
    uint32_t low, high;
    __asm__ volatile("rdmsr" : "=a"(low), "=d"(high) : "c"(msr));
    return ((uint64_t)high << 32) | low;
}

// 初始化syscall
void init_syscall(void) {
    // 设置入口地址
    cpu_write_msr(0xC0000082, (uint64_t)&syscall_entry);
    
    // 设置段
    cpu_write_msr(0xC0000081, ((uint64_t)0x1B << 48) | ((uint64_t)0x08 << 32));
    
    // 设置标志掩码
    cpu_write_msr(0xC0000084, 0x200);
    
    // 启用syscall
    uint64_t efer = cpu_read_msr(0xC0000080);
    efer |= 1;
    cpu_write_msr(0xC0000080, efer);
}