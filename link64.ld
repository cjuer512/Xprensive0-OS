/* 最简单的链接脚本 */
SECTIONS {
    .boot 0x7C00 : { boot.o(.text) }
    .loader 0x10000 : { loader.o(.text) }
    .text 0x10200 : { loader64.o(.text) }
    .data : { *(.data) }
    .bss : { *(.bss) }
}