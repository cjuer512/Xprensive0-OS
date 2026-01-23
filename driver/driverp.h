#include "stddef.h"
#include "stdint.h"

// 内联函数，用于向端口写入一个字节的数据
void outb(uint16_t port, uint8_t value);

// 内联函数，用于从端口读取一个字节的数据
uint8_t inb(uint16_t port);

// set_idt_entry64 函数声明
void set_idt_entry64(uint8_t index, uint64_t handler, uint16_t selector, uint8_t type_attr);

char get_ascii_from_set1(uint8_t scancode);
uint16_t get_cursor_position(void);
void set_cursor_position(uint16_t pos);
#ifndef IDT_ENTRY_H
#define IDT_ENTRY_H



#endif // IDT_ENTRY_H
