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
    if (word == '\n')
    {
        print_cursor_x = 0; // 回到行首
        print_cursor_y++;   // 移到下一行
        
        // 检查是否超出屏幕底部
        if (print_cursor_y >= 25)
        {
            // 超出屏幕，清屏并回到顶部
            char *video = (char *)0xB8000;
            for (int i = 0; i < 80 * 25 * 2; i += 2)
            {
                video[i] = ' ';
                video[i + 1] = 0x0F;
            }
            print_cursor_y = 0; // 回到顶部
            print_cursor_x = 0; // 行首
        }
        
        // 更新硬件光标位置
        print_set_cursor_position(print_cursor_y * 80 + print_cursor_x);
        return;
    }
    
    // 普通字符处理
    uint16_t pos = print_cursor_y * 80 + print_cursor_x;
    char *video_location = (char *)0xB8000 + pos * 2;
    video_location[0] = word;
    video_location[1] = 0x0F;
    print_cursor_x++;
    
    // 检查是否到行尾
    if (print_cursor_x >= 80)
    {
        print_cursor_x = 0;
        print_cursor_y++;
        
        // 检查是否超出屏幕底部
        if (print_cursor_y >= 25)
        {
            // 超出屏幕，清屏并回到顶部
            char *video = (char *)0xB8000;
            for (int i = 0; i < 80 * 25 * 2; i += 2)
            {
                video[i] = ' ';
                video[i + 1] = 0x0F;
            }
            print_cursor_y = 0;
            print_cursor_x = 0;
        }
    }
    
    print_set_cursor_position(print_cursor_y * 80 + print_cursor_x);
}
void print_line(char *line)
{
    for (int i = 0; line[i] != '\0'; i = i + 1)
    {
        // for(int i=0;line[i]!='\0';i = i + 1){
        print_char(line[i]);
    }
    
}