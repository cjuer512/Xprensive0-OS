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
    mov dword [0x13000], 0x00000183
    

    ; GDT必须在填充前定义！
gdt:
    ; 0. 空描述符 (必须)
    dq 0x0000000000000000

    ; 1. 64位代码段描述符 (索引 1, 选择子 0x08)
    ; 关键：L=1 (长模式), D/B=0, G=0 (对于长模式，界限通常忽略)
    ; 基址=0, 界限=0, P=1, DPL=0, S=1, Type=1010 (可执行/可读)
    dq (1 << 43) | (1 << 44) | (1 << 47) | (1 << 53)  ; 对应十六进制: 0x0020980000000000
    ; 分解: Type=1010(0xA), S=1, DPL=00, P=1 -> 访问字节 = 0x9A
    ;       标志: G=0, D/B=0, L=1, AVL=0 -> 0x20
    ;       所以高32位 = 0x00209A00，但通常直接写为 0x0020980000000000 (Type 1000也可，但1010更标准)

    ; 2. 64位数据段描述符 (索引 2, 选择子 0x10)
    ; 基址=0, 界限=0, P=1, DPL=0, S=1, Type=0010 (可读/写)
    dq (1 << 41) | (1 << 44) | (1 << 47)             ; 对应十六进制: 0x0000920000000000
    ; 访问字节 = 0x92 (P=1, DPL=00, S=1, Type=0010)
gdt64_end:

; GDT 描述符指针 (用于 LGDT 指令)
gdt_ptr:
    dw gdt64_end - gdt - 1  ; 16位界限（表大小-1）
    dq gdt 

    ; 模式切换
    lgdt [gdt_ptr]
    ;地址扩展
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

