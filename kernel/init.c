//2026.1.23
//这个文件是内核主进程，暂时就是命令行
#include "api/api.h"
#include "stdint.h"
#include "kernel.h"
#include "../driver/driverp.h"
__attribute__((noreturn)) void kernel_init(){
    char *video_init = (char *)0xB8000;
    for (int i = 0; i < 80 * 25 * 2; i += 2)
    {
        video_init[i] = ' ';
        video_init[i + 1] = 0x0F; // 白字黑底
    }
    
    print_line("hello,world\n");
    
    sucmd_main();
    while(1) {
        // 处理任务、中断等
        __asm__ volatile("hlt");  // 节能的 halt 指令
    }
}