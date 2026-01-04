/* 最简单的链接脚本 */
SECTIONS {
    .boot 0x7C00 : { boot.o(.text) }
    .loader 0x10000 : { loader.o(.text) }
    .text 0x10200 : { loader64.o(.text) }
    .keyboard_driver 0x11000 : {
        /* 确保该段的内容只来自键盘驱动目标文件 */
        driver/keyboard.o(.text)
        driver/keyboard.o(.data)
        driver/keyboard.o(.rodata)
    }
    .data : { *(.data) }
    .bss : { *(.bss) }
}