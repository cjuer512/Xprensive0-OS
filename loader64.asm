[bits 64]
org 0x10200    ; 加载到 0x10200

    ; 显示"64"
    mov dword [0xB8000+4], 0x1F361F34
    jmp $

; 填充到512字节  
times 510-($-$$) db 0
dw 0x1111