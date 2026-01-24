// kernel/init.c
#include "api/api.h"
#include "stdint.h"
#include "kernel.h"
#include "../driver/driverp.h"
#include "set/set.h"

// 测试系统调用
static void test_syscall(void) {
    extern void syscall_entry(void);

    
    print_line("Testing syscall directly...\n");
    
    // 直接调用syscall_entry，看看它是否能工作
    syscall_entry();
    
    print_line("Direct call completed\n");
}

__attribute__((noreturn)) void kernel_init(){
    init_syscall();
    
    char *video_init = (char *)0xB8000;
    for (int i = 0; i < 80 * 25 * 2; i += 2) {
        video_init[i] = ' ';
        video_init[i + 1] = 0x0F;
    }
    
    print_line("hello,world\n");
    
    // 测试syscall
    test_syscall();
    
    sucmd_main();
    
    while(1) {
        __asm__ volatile("hlt");
    }
}