#include "stdint.h"

static uint16_t print_cursor_x = 0; // 当前列 (0-79)
static uint16_t print_cursor_y = 0; // 当前行 (0-24)

uint16_t print_get_cursor_position(void)
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

    return pos;
}

void print_set_cursor_position(uint16_t pos)
{
    // 设置高字节
    __asm__ volatile("outb %1, %0" : : "dN"((uint16_t)0x3D4), "a"((uint8_t)0x0E));
    __asm__ volatile("outb %1, %0" : : "dN"((uint16_t)0x3D5), "a"((uint8_t)((pos >> 8) & 0xFF)));

    // 设置低字节
    __asm__ volatile("outb %1, %0" : : "dN"((uint16_t)0x3D4), "a"((uint8_t)0x0F));
    __asm__ volatile("outb %1, %0" : : "dN"((uint16_t)0x3D5), "a"((uint8_t)(pos & 0xFF))); // ⚠️ 修正：删除 "initial"
}

void print_char(char word)
{
    if (word == '\n') {
        print_cursor_x = 0;        // 回到行首
        print_cursor_y++;          // 移到下一行
        if (print_cursor_y >= 25) {
            print_cursor_y = 0;    // 简单滚屏：回到顶部
        }
        // 更新硬件光标位置
        print_set_cursor_position(print_cursor_y * 80 + print_cursor_x);
        return; // 换行符不需要写入显存，直接返回
    }
    char *video = (char *)0xB8000;
    char *buffer = (char *)0x8800;

    // 清屏（可选）

    print_get_cursor_position();
    uint16_t pos = print_cursor_y * 80 + print_cursor_x; // 计算位置
    char *video_locaiton = (char *)0xB8000 + pos * 2;    // 定位到正确位置
    video_locaiton[0] = word;
    video_locaiton[1] = 0x0F;
    print_cursor_x++; // 列加1

    if (print_cursor_x >= 80)
    {                       // 如果到行尾
        print_cursor_x = 0; // 回到行首s
        print_cursor_y++;   // 换到下一行
        if (print_cursor_y >= 25)
        {                       // 如果到底部
            print_cursor_y = 0; // 回到顶部（简单处理）
        }
    }
    print_set_cursor_position(print_cursor_y * 80 + print_cursor_x);
}
void print_line(char *line)
{
    for (int i = 0; line[i]!='\0'; i = i + 1)
    {
        // for(int i=0;line[i]!='\0';i = i + 1){
        print_char(line[i]);
    }
    
}