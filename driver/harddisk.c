/*
 * driver/hdd_full.c
 * 完整硬盘驱动 - 一个文件搞定
 * 主要功能：IDT设置 + PIC配置 + 中断处理 + 基本读写
 */

#include "stdint.h"
#include "driverp.h"
// ========== 端口定义 ==========
#define ATA_DATA        0x1F0
#define ATA_ERROR       0x1F1
#define ATA_SECTOR_CNT  0x1F2
#define ATA_SECTOR_NUM  0x1F3
#define ATA_CYL_LOW     0x1F4
#define ATA_CYL_HIGH    0x1F5
#define ATA_DRIVE_HEAD  0x1F6
#define ATA_STATUS      0x1F7
#define ATA_COMMAND     0x1F7
#define ATA_ALT_STATUS  0x3F6

// 状态位
#define ATA_SR_BSY      0x80
#define ATA_SR_DRDY     0x40
#define ATA_SR_DF       0x20
#define ATA_SR_DRQ      0x08
#define ATA_SR_ERR      0x01

// 命令
#define ATA_CMD_READ_PIO    0x20
#define ATA_CMD_WRITE_PIO   0x30
#define ATA_CMD_IDENTIFY    0xEC

// ========== 全局状态 ==========
static volatile int hdd_irq_flag = 0;
static uint16_t* hdd_buffer_ptr = 0;
static uint8_t hdd_sector_count = 0;
static uint8_t hdd_current_sector = 0;

// ========== I/O函数 ==========


static inline uint16_t inw(uint16_t port) {
    uint16_t ret;
    __asm__ volatile("inw %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline void outw(uint16_t port, uint16_t value) {
    __asm__ volatile("outw %0, %1" : : "a"(value), "Nd"(port));
}

// ========== 中断处理函数 ==========
__attribute__((interrupt))
static void hdd_irq_handler(void* frame) {
    (void)frame;
    
    // 清除中断
    (void)inb(ATA_STATUS);
    
    // 如果正在读取数据，读取当前扇区
    if (hdd_buffer_ptr && hdd_sector_count > 0) {
        for (int i = 0; i < 256; i++) {
            hdd_buffer_ptr[hdd_current_sector * 256 + i] = inw(ATA_DATA);
        }
        hdd_current_sector++;
    }
    
    // 设置完成标志
    if (hdd_current_sector >= hdd_sector_count) {
        hdd_irq_flag = 1;
        hdd_buffer_ptr = 0;
        hdd_sector_count = 0;
        hdd_current_sector = 0;
    }
    
    // 双重EOI
    outb(0xA0, 0x20);
    outb(0x20, 0x20);
}

// ========== 函数1：初始化硬盘驱动 ==========
void hdd_init_all(uint8_t* idt) {
    /* 
     * 初始化硬盘驱动（IDT + PIC）
     * 参数：idt - IDT表地址
     */
    
    // 1. 设置IDT条目（中断0x2E）
    uint64_t handler = (uint64_t)hdd_irq_handler;
    uint8_t* entry = idt + (0x2E * 16);
    
    *(uint16_t*)(entry + 0) = handler & 0xFFFF;
    *(uint16_t*)(entry + 2) = 0x0008;
    *(uint8_t*)(entry + 4) = 0x00;
    *(uint8_t*)(entry + 5) = 0x8E;
    *(uint16_t*)(entry + 6) = (handler >> 16) & 0xFFFF;
    *(uint32_t*)(entry + 8) = handler >> 32;
    *(uint32_t*)(entry + 12) = 0;
    
    // 2. 配置PIC允许硬盘中断
    uint8_t master = inb(0x21);
    uint8_t slave = inb(0xA1);
    master &= ~(1 << 2);  // 允许IRQ2
    slave &= ~(1 << 6);   // 允许IRQ14
    outb(0x21, master);
    outb(0xA1, slave);
    
    // 3. 清空可能的待处理中断
    while (inb(0x64) & 0x01) {
        inb(0x60);
    }
}

// ========== 函数2：读取硬盘扇区 ==========
int hdd_read(uint32_t lba, uint8_t count, uint16_t* buffer) {
    /*
     * 读取硬盘扇区
     * 参数：lba - 起始扇区号，count - 扇区数，buffer - 数据缓冲区
     * 返回：0=成功，-1=失败
     */
    
    // 等待硬盘就绪
    while (inb(ATA_STATUS) & ATA_SR_BSY) {}
    
    // 设置全局变量供中断使用
    hdd_buffer_ptr = buffer;
    hdd_sector_count = count;
    hdd_current_sector = 0;
    hdd_irq_flag = 0;
    
    // 发送LBA地址和扇区数
    outb(ATA_DRIVE_HEAD, 0xE0 | ((lba >> 24) & 0x0F));
    outb(ATA_SECTOR_CNT, count);
    outb(ATA_SECTOR_NUM, lba & 0xFF);
    outb(ATA_CYL_LOW, (lba >> 8) & 0xFF);
    outb(ATA_CYL_HIGH, (lba >> 16) & 0xFF);
    
    // 发送读命令
    outb(ATA_COMMAND, ATA_CMD_READ_PIO);
    
    // 等待所有扇区读取完成
    while (!hdd_irq_flag) {
        __asm__ volatile("pause");
    }
    
    return 0;
}

// ========== 函数3：简单读扇区（轮询，不依赖中断） ==========
int hdd_read_simple(uint32_t lba, uint16_t* buffer) {
    /*
     * 简单读取一个扇区（轮询方式）
     * 用于测试或不需要中断的场景
     */
    
    // 等待就绪
    while (inb(ATA_STATUS) & ATA_SR_BSY) {}
    
    // 发送LBA地址（读一个扇区）
    outb(ATA_DRIVE_HEAD, 0xE0 | ((lba >> 24) & 0x0F));
    outb(ATA_SECTOR_CNT, 1);
    outb(ATA_SECTOR_NUM, lba & 0xFF);
    outb(ATA_CYL_LOW, (lba >> 8) & 0xFF);
    outb(ATA_CYL_HIGH, (lba >> 16) & 0xFF);
    
    // 发送读命令
    outb(ATA_COMMAND, ATA_CMD_READ_PIO);
    
    // 等待数据就绪
    uint8_t status;
    do {
        status = inb(ATA_STATUS);
        if (status & ATA_SR_ERR) return -1;
    } while (!(status & ATA_SR_DRQ));
    
    // 读取512字节数据
    for (int i = 0; i < 256; i++) {
        buffer[i] = inw(ATA_DATA);
    }
    
    return 0;
}