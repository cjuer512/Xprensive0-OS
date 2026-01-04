#!/bin/bash
rm -f hd.raw *.bin *.elf *.o

# 1. 创建虚拟硬盘
dd if=/dev/zero of=hd.raw bs=512 count=6144 2>/dev/null

# 2. boot.asm (第一扇区, 0x7C00)
nasm -f bin -o boot.bin bootloader/boot.asm
truncate -s 512 boot.bin
dd if=boot.bin of=hd.raw bs=512 count=1 conv=notrunc

# 3. loader.asm (第二扇区, 运行在0x10000)
nasm -f bin -o loader.bin bootloader/loader.asm
truncate -s 512 loader.bin
dd if=loader.bin of=hd.raw bs=512 seek=1 conv=notrunc

# 4. 编译所有C文件
echo "=== 编译C文件 ==="
gcc -c -m64 -ffreestanding -nostdlib -fno-builtin \
    -mno-red-zone -mgeneral-regs-only \
    bootloader/loader64.c -o loader64.o

gcc -c -m64 -ffreestanding -nostdlib -fno-builtin \
    -mno-red-zone -mgeneral-regs-only \
    driver/keyboard.c -o keyboard.o

# 5. 关键修改：将两个.o文件一起链接到0x10200
echo "=== 链接（loader64 + keyboard）==="
ld -nostdlib -Ttext=0x10200 -o kernel.elf loader64.o keyboard.o

# 6. 提取.text段（现在包含两个模块的代码）
objcopy -O binary --only-section=.text kernel.elf kernel.bin

# 7. 验证提取的内容
echo "kernel.bin大小: $(stat -c%s kernel.bin) 字节"
hexdump -C kernel.bin | head -5

# 8. 写入磁盘（从扇区2开始）
dd if=kernel.bin of=hd.raw bs=512 seek=2 conv=notrunc

# 9. 验证磁盘写入
echo "=== 磁盘验证 ==="
echo "扇区2开头（应该非零）:"
dd if=hd.raw bs=512 skip=2 count=1 2>/dev/null | hexdump -C | head -3

# 10. 生成符号表用于调试
echo "=== 符号地址 ==="
nm kernel.elf | grep -E "(keyboard_handler|keyboard_init|setup_keyboard_interrupt)"