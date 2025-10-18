BITS 16
ORG 0x7C00

start:
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00
    sti

    ; 显示启动消息
    mov si, msg1
    call print_str

    ; 启用A20
    in al, 0x92
    or al, 2
    out 0x92, al

    ; 加载GDT
    lgdt [gdt_desc]

    ; 进入保护模式
    mov eax, cr0
    or eax, 1
    mov cr0, eax

    ; 远跳转
    jmp 0x08:protected_mode

print_str:
    mov ah, 0x0E
.loop:
    lodsb
    test al, al
    jz .done
    int 0x10
    jmp .loop
.done:
    ret

; 32位保护模式
BITS 32
protected_mode:
    ; 设置段寄存器
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x7C00

    ; 禁用中断
    cli

    ; 显示保护模式消息
    mov esi, msg2
    mov edi, 0xB8000
    mov ah, 0x0F
    call print_32

    ; 设置分页
    call setup_paging

    ; 启用PAE
    mov eax, cr4
    or eax, (1 << 5)    ; PAE
    mov cr4, eax

    ; 设置EFER.LME
    mov ecx, 0xC0000080
    rdmsr
    or eax, (1 << 8)    ; LME
    wrmsr

    ; 启用分页
    mov eax, cr0
    or eax, (1 << 31)   ; PG
    mov cr0, eax

    ; 加载64位GDT
    lgdt [gdt64_desc]

    ; 跳转到64位模式
    jmp 0x08:long_mode

setup_paging:
    ; 清除页表区域 (0x1000-0x4000)
    mov edi, 0x1000
    mov ecx, 0x1000
    xor eax, eax
    rep stosd

    ; 设置PML4 (0x1000)
    mov dword [0x1000], 0x2003  ; 指向PDPT

    ; 设置PDPT (0x2000)
    mov dword [0x2000], 0x3003  ; 指向PD

    ; 设置PD (0x3000) - 2MB页映射前2MB内存
    mov dword [0x3000], 0x00000083  ; 2MB页标志

    ; 设置CR3
    mov eax, 0x1000
    mov cr3, eax
    ret

print_32:
    mov edi, 0xB8000
.print_loop:
    mov al, [esi]
    test al, al
    jz .done
    mov [edi], ax
    add edi, 2
    inc esi
    jmp .print_loop
.done:
    ret

; 64位长模式
BITS 64
long_mode:
    ; 设置段寄存器
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov rsp, 0x7C00

    ; 显示64位模式消息
    mov rsi, msg3
    mov rdi, 0xB8000 + 160  ; 第二行显示
    mov ah, 0x0E
    call print_64

.hang:
    hlt
    jmp .hang

print_64:
.print_loop:
    mov al, [rsi]
    test al, al
    jz .done
    mov [rdi], ax
    add rdi, 2
    inc rsi
    jmp .print_loop
.done:
    ret

; 数据区
BITS 16
msg1 db "Booting...", 0
msg2 db "32-bit Protected Mode", 0
msg3 db "64-bit Long Mode Active!", 0

; 32位GDT
gdt_start:
    dq 0x0000000000000000
gdt_code:
    dw 0xFFFF
    dw 0x0000
    db 0x00
    db 0x9A
    db 0xCF
    db 0x00
gdt_data:
    dw 0xFFFF
    dw 0x0000
    db 0x00
    db 0x92
    db 0xCF
    db 0x00
gdt_end:

gdt_desc:
    dw gdt_end - gdt_start - 1
    dd gdt_start

; 64位GDT
gdt64_start:
    dq 0x0000000000000000
gdt64_code:
    dw 0x0000
    dw 0x0000
    db 0x00
    db 0x9A
    db 0x20
    db 0x00
gdt64_data:
    dw 0x0000
    dw 0x0000
    db 0x00
    db 0x92
    db 0x00
    db 0x00
gdt64_end:

gdt64_desc:
    dw gdt64_end - gdt64_start - 1
    dd gdt64_start

times 510-($-$$) db 0
dw 0xAA55
