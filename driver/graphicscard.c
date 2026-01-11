// graphicscard.c - 最简单的显卡驱动

#include "stdint.h"
#include "driverp.h"

// 1. VGA模式13h的显存地址
#define VGA_MEMORY 0xA0000


// 2. VGA寄存器端口
#define VGA_AC_INDEX   0x3C0
#define VGA_MISC_WRITE 0x3C2
#define VGA_SEQ_INDEX  0x3C4
#define VGA_SEQ_DATA   0x3C5
#define VGA_CRTC_INDEX 0x3D4
#define VGA_CRTC_DATA  0x3D5
#define VGA_GC_INDEX   0x3CE
#define VGA_GC_DATA    0x3CF
#define VGA_AC_WRITE   0x3C0
#define VGA_AC_READ    0x3C1

// ========== 函数1：设置VGA模式13h ==========
void vga_set_mode_13h(void) {
    uint8_t *vga_buffer = (uint8_t*)VGA_MEMORY;
    // 设置320x200 256色模式
    outb(VGA_MISC_WRITE, 0x63);
    
    // 序列器寄存器
    outb(VGA_SEQ_INDEX, 0); outb(VGA_SEQ_DATA, 0x03);
    outb(VGA_SEQ_INDEX, 1); outb(VGA_SEQ_DATA, 0x01);
    outb(VGA_SEQ_INDEX, 2); outb(VGA_SEQ_DATA, 0x0F);
    outb(VGA_SEQ_INDEX, 3); outb(VGA_SEQ_DATA, 0x00);
    outb(VGA_SEQ_INDEX, 4); outb(VGA_SEQ_DATA, 0x0E);
    
    // 解锁CRTC
    outb(VGA_CRTC_INDEX, 0x03); outb(VGA_CRTC_DATA, inb(VGA_CRTC_DATA) | 0x80);
    outb(VGA_CRTC_INDEX, 0x11); outb(VGA_CRTC_DATA, inb(VGA_CRTC_DATA) & ~0x80);
    
    // CRTC寄存器（设置分辨率）
    uint8_t crtc[] = {0x5F,0x4F,0x50,0x82,0x54,0x80,0xBF,0x1F,0x00,0x41,0x00,0x00,0x00,0x00,0x00,0x00,0x9C,0x0E,0x8F,0x28,0x40,0x96,0xB9,0xA3,0xFF};
    for(int i=0; i<25; i++) { outb(VGA_CRTC_INDEX, i); outb(VGA_CRTC_DATA, crtc[i]); }
    
    // 图形控制器
    uint8_t gc[] = {0x00,0x00,0x00,0x00,0x00,0x40,0x05,0x0F,0xFF};
    for(int i=0; i<9; i++) { outb(VGA_GC_INDEX, i); outb(VGA_GC_DATA, gc[i]); }
    
    // 属性控制器
    inb(VGA_AC_READ);  // 重置
    uint8_t ac[] = {0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,0x41,0x00,0x0F,0x00,0x00};
    for(int i=0; i<21; i++) { outb(VGA_AC_INDEX, i); outb(VGA_AC_WRITE, ac[i]); }
    
    inb(VGA_AC_READ);
    outb(VGA_AC_INDEX, 0x20);
}

// ========== 函数2：设置调色板颜色 ==========
void vga_set_palette_color(uint8_t index, uint8_t r, uint8_t g, uint8_t b) {
    uint8_t *vga_buffer = (uint8_t*)VGA_MEMORY;
    outb(0x3C8, index);      // 设置调色板索引
    outb(0x3C9, r >> 2);     // 红色 (VGA使用6位，所以右移2位)
    outb(0x3C9, g >> 2);     // 绿色
    outb(0x3C9, b >> 2);     // 蓝色
}

// ========== 函数3：画一个像素 ==========
void vga_put_pixel(int x, int y, uint8_t color) {
    uint8_t *vga_buffer = (uint8_t*)VGA_MEMORY;
    if(x >= 0 && x < 320 && y >= 0 && y < 200) {
        vga_buffer[y * 320 + x] = color;
    }
}

// ========== 函数4：清屏 ==========
void vga_clear_screen(uint8_t color) {
    uint8_t *vga_buffer = (uint8_t*)VGA_MEMORY;
    for(int i = 0; i < 320 * 200; i++) {
        vga_buffer[i] = color;
    }
}