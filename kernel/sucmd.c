#include "api/api.h"
#include "stdint.h"
#include "../driver/driverp.h"
static int i = 0;
static uint16_t pos = 0;

// 新增：命令状态标志
static volatile int command_ready = 0; // 1=命令已输入，等待执行
static char command_buffer[256] = {0}; // 存储命令字符串
static int cmd_length = 0;
// 命令执行函数（具体实现）
int cmpstr(char* start,char* targetword,int number){
    for(int i = 0;i<number;i = i+1){
        if(start[i]==targetword[i]){
            if(i == number-1){
                return 1;
            }
            continue;
        }else{
            return 0;
        }
    }
}
int len(const char *str) {
    int count = 0;
    while (str[count] != '\0') {
        count++;
    }
    return count;
}
void execute_command(char *cmd)
{
    if (cmd[0] == '\0')
    {
        print_line("(empty command)\n");
        return;
    }

    print_line("\n[Executing: ");
    print_line(cmd);
    print_line("]\n");

    // 示例命令实现
    // 检查前几个字符即可，避免复杂的字符串比较
    if (cmd[0] == 'h' && cmd[1] == 'e' && cmd[2] == 'l' && cmd[3] == 'p' && cmd[4] == '\0')
    {
        print_line("Available commands:\n");
        print_line("  help    - Show this help\n");
        print_line("  clear   - Clear screen\n");
        print_line("  echo    - Echo text\n");
    }
    else if (cmd[0] == 'c' && cmd[1] == 'l' && cmd[2] == 'e' && cmd[3] == 'a' && cmd[4] == 'r' && cmd[5] == '\0')
    {
        // 清屏
        char *video = (char *)0xB8000;
        for (int i = 0; i < 80 * 25 * 2; i += 2)
        {
            video[i] = ' ';
            video[i + 1] = 0x0F;
        }
        pos = 0;
        set_cursor_position(0);
    }
    else if (cmd[0] == 'e' && cmd[1] == 'c' && cmd[2] == 'h' && cmd[3] == 'o' && cmd[4] == ' ' && cmd[5] != '\0')
    {
        // echo命令：显示后面的内容
        print_line(&cmd[5]); // 跳过 "echo "
        print_line("\n");
    }else if(cmpstr(cmd,"version",len("version"))){
        int a = len("version");
        char b = (char)a;
        print_char(b);
        print_line("Xprensive-OS,version0.0.0,author:cjuer512,chinajuer2009@outlook.com");
        print_line("\n");
    }else if(cmpstr(cmd,"diskread",8))
    {
        __asm__ volatile("movq $0,%%rax":::"rax");
        __asm__ volatile("movl $1,%%edi":::"rdi");
        __asm__ volatile("movb $1,%%sil":::"rsi");
        __asm__ volatile("movq $0x20000,%%rdx":::"rdx");
        __asm__ volatile("int $0x2E");
        print_line("ok");
    }
    else
    {
        print_line("Unknown command. Type 'help' for list.\n");
    }
}
void input()
{
    uint8_t scancode = inb(0x60);
    char ascii_code = get_ascii_from_set1(scancode);

    // 只处理按键按下
    if (scancode >= 0x80 || ascii_code == 0)
    {
        outb(0x20, 0x20);
        return;
    }

    // 回车键：设置命令就绪标志
    if (ascii_code == '\n')
    {
        // 1. 命令缓冲区添加结束符
        if (cmd_length < 255)
        {
            command_buffer[cmd_length] = '\0';
        }

        // 2. 关键：设置命令就绪标志
        command_ready = 1;

        // 3. 换行
        pos = get_cursor_position();
        uint16_t next_line = (pos / 80 + 1) * 80;

        set_cursor_position(next_line);

        outb(0x20, 0x20);
        return;
    }

    // 退格键处理
    if (ascii_code == '\b')
    {
        if (cmd_length > 0)
        {
            cmd_length--;

            // 从屏幕删除
            pos = get_cursor_position();
            if (pos > 0)
            {
                pos--;
                char *video_location = (char *)0xB8000 + pos * 2;
                video_location[0] = ' ';
                set_cursor_position(pos);
            }
        }
        outb(0x20, 0x20);
        return;
    }

    // 普通字符：存入缓冲区和显示
    if (cmd_length < 255)
    {
        command_buffer[cmd_length] = ascii_code;
        cmd_length++;

        // 显示到屏幕
        pos = get_cursor_position();

        // 现在用正确的pos计算显示位置
        char *video_location = (char *)0xB8000 + pos * 2;
        video_location[0] = ascii_code;
        video_location[1] = 0x0F;

        // 光标右移
        pos++;
        set_cursor_position(pos);
    }

    outb(0x20, 0x20);
}

void sucmd_main()
{
    print_line("hello,welcome to Xprensive-OS command prompt\n");
    print_line(">>>");

    // 将 input 函数的地址存入固定位置 0x9000
    uint64_t *keyboard_new_handler_positon = (uint64_t *)0x9000;
    *keyboard_new_handler_positon = (uint64_t)input;

    // 新增：主循环，等待并执行命令
    while (1)
    {
        // 1. 检查命令是否就绪
        if (command_ready)
        {
            // 2. 执行命令
            execute_command(command_buffer);

            // 3. 重置状态，准备下一条命令
            command_ready = 0;
            cmd_length = 0;

            pos = get_cursor_position();

            // 先检查是否需要清屏！！！

            // 4. 显示新的提示符
            print_line("\n>>>");
        }

        // 5. 让出CPU（重要！）
        __asm__ volatile("hlt");
    }
}