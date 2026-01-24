[org 0x7c00]
[bits 16]


start:
    ; 1. 初始化段寄存器
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7a00
    mov al, 0x03
    mov ah, 0x00
    int 0x10
    ; 显示启动信息
    mov si, boot_msg
    call print_string
    mov ax, 0x1000      ; 段地址: 0x1000
    mov es, ax
    xor bx, bx          ; 偏移: 0x0000
    
    mov ah, 0x02        ; 读扇区功能
    mov al, 32           ; 读取20个扇区(2KB)
    mov ch, 0           ; 柱面0
    mov cl, 2           ; 从第2扇区开始
    mov dh, 0           ; 磁头0
    mov dl, 0x80           ; 驱动器0
    int 0x13             ; 如果出错(进位标志=1)
    jc disk_error

    ; 2. 进入保护模式
    call switch_to_pm
    
    ; 这里不会执行，因为已经跳转到32位代码
    jmp $

; ==================== 16位实模式函数 ====================
print_string:
    mov ah, 0x0E
.loop:
    lodsb
    cmp al, 0
    je .done
    int 0x10
    jmp .loop
.done:
    ret

disk_error:

    mov si, disk_err_msg
    call print_string
    jmp $

; ==================== 切换到保护模式 ====================
switch_to_pm:

    cli                     ; 1. 禁用中断
    lgdt [gdt_descriptor]   ; 2. 加载GDT

    mov eax, cr0            ; 3. 设置保护模式位
    or eax, 0x1
    mov cr0, eax

    jmp CODE_SEG:init_pm    ; 4. 远跳转到32位代码

; ==================== GDT 定义 ====================
gdt_start:
    ; 空描述符（必须的）
    gdt_null:
        dd 0x0
        dd 0x0

    ; 代码段描述符
    gdt_code:
        ; 基地址=0, 界限=0xfffff
        ; 标志:  Present=1, Privilege=00, Descriptor type=1
        ; 类型标志: Code=1, Conforming=0, Readable=1, Accessed=0
        dw 0xffff       ; 界限 (0-15)
        dw 0x0          ; 基地址 (0-15)
        db 0x0          ; 基地址 (16-23)
        db 10011010b    ; 访问字节
        db 11001111b    ; 标志 + 界限 (16-19)
        db 0x0          ; 基地址 (24-31)

    ; 数据段描述符
    gdt_data:
        ; 类型标志: Code=0, Expand down=0, Writable=1, Accessed=0
        dw 0xffff       ; 界限 (0-15)
        dw 0x0          ; 基地址 (0-15)
        db 0x0          ; 基地址 (16-23)
        db 10010010b    ; 访问字节
        db 11001111b    ; 标志 + 界限 (16-19)
        db 0x0          ; 基地址 (24-31)

gdt_end:

; GDT 描述符
gdt_descriptor:
    dw gdt_end - gdt_start - 1  ; GDT大小
    dd gdt_start                 ; GDT起始地址

; 段选择子
CODE_SEG equ gdt_code - gdt_start
DATA_SEG equ gdt_data - gdt_start

; ==================== 32位保护模式代码 ====================
[bits 32]
init_pm:
    ; 5. 更新段寄存器
    mov ax, 0x10
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    ; 6. 更新栈指针
    mov ebp, 0x90000
    mov esp, ebp

    ; 7. 调用32位主函数
    call BEGIN_PM
; 32位保护模式的主函数
BEGIN_PM:
    ; 在这里，你已经完全运行在32位保护模式下！
    ; 可以访问4GB内存，使用32位寄存器等
    
    ; 示例：在屏幕上显示信息（使用显存）
    mov esi, pm_msg
   
    mov edi, 0xb8000     ; 文本显存地址
    mov ah, 0x0f         ; 白字黑底
    
    

.print_loop:
    mov al, [esi]
    cmp al, 0
    je .done
    mov [edi], ax        ; 写入字符和属性
    add edi, 2
    inc esi
    jmp .print_loop

.done:
    jmp 0x10000
    
; ==================== 数据区 ====================
[bits 16]
boot_msg db "Starting in 16-bit Real Mode...", 0x0D, 0x0A, 0
pm_msg db "Successfully entered 32-bit Protected Mode!", 0
disk_err_msg db "Disk read error!", 0
; ==================== 引导扇区填充 ====================
times 510-($-$$) db 0
dw 0xaa55