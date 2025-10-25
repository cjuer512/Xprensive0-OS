[bits 32]
org 0x10000

_start:
    ; 基础设置
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov esp, 0x90000

    ; 显示"32"
    mov dword [0xB8000], 0x1F331F32

    ; 设置页表
    mov edi, 0x11000
    mov cr3, edi
    mov dword [edi], 0x12003
    mov dword [0x12000], 0x13003
    mov dword [0x13000], 0x00083

    ; GDT必须在填充前定义！
gdt:
    dq 0
    dq 0x0020980000000000
    dq 0x0000920000000000
gdt_ptr:
    dw 23
    dd gdt

    ; 模式切换
    lgdt [gdt_ptr]
    mov eax, cr4
    or eax, (1 << 5)
    mov cr4, eax
    mov ecx, 0xC0000080
    rdmsr
    or eax, (1 << 8)
    wrmsr
    mov eax, cr0
    or eax, (1 << 31)
    mov cr0, eax

    ; 跳转到阶段2
    jmp 0x08:0x10200

; 填充到512字节（使用较小值）
times 450-($-$$) db 0  ; 先填一部分
dw 0xAA55