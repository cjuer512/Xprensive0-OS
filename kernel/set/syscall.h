// 在某个头文件（如 syscall.h）中定义
#include "stdint.h"
struct syscall_regs {
    uint64_t rax; // 系统调用号
    uint64_t rdi; // 参数1
    uint64_t rsi; // 参数2
    uint64_t rdx; // 参数3
    uint64_t r10; // 参数4（syscall用r10）
    uint64_t r8;  // 参数5
    uint64_t r9;  // 参数6
    uint64_t rcx; // 用户RIP（syscall自动保存）
    uint64_t r11; // 用户RFLAGS（syscall自动保存）
};

// 声明一个全局实例
extern struct syscall_regs current_syscall_regs;