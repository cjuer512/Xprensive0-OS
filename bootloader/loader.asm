[bits 32]
org 0x10000
MagicnumberLoader db 'cjuerLOADER'
_start:
    ; 基础设置
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov esp, 0x90000

    ; 显示"INIT"
    mov dword [0xB8000], 0x1F491F4E
    mov dword [0xB8004], 0x1F541F49

    ; 简单但有效的页表设置
    ; 只映射最低的2MB，包括我们的代码和数据
    
    ; 1. 清空PML4
    mov edi, 0x8000
    xor eax, eax
    mov ecx, 1024
    rep stosd
    
    ; 2. 设置PML4[0] -> PDPT at 0x9000
    mov dword [0x8000], 0x9003
    mov dword [0x8004], 0
    
    ; 3. 清空PDPT
    mov edi, 0x9000
    mov ecx, 1024
    rep stosd
    
    ; 4. 设置PDPT[0] -> PDT at 0xA000
    mov dword [0x9000], 0xA003
    mov dword [0x9004], 0
    
    ; 5. 清空PDT
    mov edi, 0xA000
    mov ecx, 1024
    rep stosd
    
    ; 6. 映射第一个2MB页 (包括0x00000-0x1FFFFF)
    mov dword [0xA000], 0x00000183  ; 物理地址0x0, 2MB页
    mov dword [0xA004], 0
    
    ; 7. 映射视频内存区域 (0xB8000在物理地址0xB8000)
    ; 0xB8000在第二个2MB页中
    mov dword [0xA008], 0x00200183  ; 物理地址0x200000, 2MB页
    mov dword [0xA00C], 0
    
    ; 8. 设置CR3
    mov eax, 0x8000
    mov cr3, eax
    
    ; 显示"PAGE"
    mov dword [0xB8008], 0x1F451F50
    mov dword [0xB800C], 0x1F001F47

    ; 启用PAE
    mov eax, cr4
    or eax, (1 << 5)
    mov cr4, eax
    
    ; 设置LME
    mov ecx, 0xC0000080
    rdmsr
    or eax, (1 << 8)
    wrmsr
    
    ; 加载GDT
    lgdt [gdt_ptr]
    
    ; 启用分页
    mov eax, cr0
    or eax, (1 << 31)
    mov cr0, eax
    
    ; 跳转到64位
    jmp 0x08:long_mode_entry

gdt:
    dq 0x0000000000000000      ; 空描述符
    dq 0x00209A0000000000      ; 64位代码段：可执行、代码段
    dq 0x0000920000000000      ; 64位数据段
gdt_end:

gdt_ptr:
    dw gdt_end - gdt - 1
    dd gdt

[bits 64]
long_mode_entry:
    ; 显示"64"
    mov rax, 0x1F341F36
    mov [0xB8010], rax
    
    ; 设置段寄存器
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov r8,$
    jmp cmp_cjuer
    ;jmp 0x10200

    
    

cmp_cjuer:
    add r8,1
    cmp byte [r8],0x63
    jne cmp_cjuer
    cmp byte [r8+1],0x6A
    jne cmp_cjuer
    cmp byte [r8+2],0x75
    jne cmp_cjuer
    cmp byte [r8+3],0x65
    jne cmp_cjuer
    cmp byte [r8+4],0x72
    jne cmp_cjuer
    mov rax, r8      ; rax = 魔术地址
    add rax, 6       ; rax = 魔术地址 + 6
    jmp rax          ; 跳转到rax指向的地址

times 512 - ($ - $$) db 0