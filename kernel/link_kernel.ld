/* link_kernel.ld - 完整的内存布局 */
ENTRY(sucmd_main)

SECTIONS {
    .text.boot : {
        *(.text.boot) /* 收集所有标记为 .text.boot 的代码 */
    }
    /* 代码段 */
    .text : {
        *(.text)
        *(.text.*)
    }
    
    /* 只读数据段（字符串在这里！） */
    . = ALIGN(4096);
    .rodata : {
        *(.rodata)
        *(.rodata.*)
    }
    
    /* 可读写数据段 */
    . = ALIGN(4096);
    .data : {
        *(.data)
        *(.data.*)
    }
    
    /* 未初始化数据段 (BSS) */
    . = ALIGN(4096);
    .bss : {
        __bss_start = .;
        *(.bss)
        *(.bss.*)
        *(COMMON)
        __bss_end = .;
    }
    
    /* 栈空间（可选） */
    . = ALIGN(4096);
    .stack : {
        . = . + 0x4000; /* 16KB栈 */
        __stack_top = .;
    }
    
    /* 丢弃其他段 */
    /DISCARD/ : {
        *(.comment)
        *(.eh_frame)
    }
}