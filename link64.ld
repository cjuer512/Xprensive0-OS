/* 最简单的链接脚本 */
SECTIONS {
    .boot 0x7C00 : { boot.o(.text) }
    .loader 0x10000 : { loader.o(.text) }
    .text 0x10200 : { loader64.o(.text) }
    .keyboard_driver 0x20000 : {
        /* 确保该段的内容只来自键盘驱动目标文件 */
        keyboard.o(.text)
        keyboard.o(.data)
        keyboard.o(.rodata)
        driverp.o(.text)
        driverp.o(.data)
        driverp.o(.rodata)
        harddisk.o(.text)
        harddisk.o(.data)
        harddisk.o(.rodata)
        
        graphicscard.o(.text)
        graphicscard.o(.data)
        graphicscard.o(.rodata)
    }
    .data : { *(.data) }
    .bss : { *(.bss) }
}