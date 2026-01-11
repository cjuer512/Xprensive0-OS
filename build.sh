#!/bin/bash
rm -f hd.raw *.bin *.elf *.o

# 1. 创建虚拟硬盘
dd if=/dev/zero of=hd.raw bs=512 count=16384 2>/dev/null

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

gcc -c -m64 -ffreestanding -nostdlib -fno-builtin \
    -mno-red-zone -mgeneral-regs-only \
    api/print.c -o print.o

gcc -c -m64 -ffreestanding -nostdlib -fno-builtin \
    -mno-red-zone -mgeneral-regs-only \
    driver/driverp.c -o driverp.o

gcc -c -m64 -ffreestanding -nostdlib -fno-builtin \
    -mno-red-zone -mgeneral-regs-only \
    driver/harddisk.c -o harddisk.o

gcc -c -m64 -ffreestanding -nostdlib -fno-builtin \
    -mno-red-zone -mgeneral-regs-only \
    driver/graphicscard.c -o graphicscard.o
# 5. 关键修改：将两个.o文件一起链接到0x10200
echo "=== 链接（loader64 + keyboard）==="
ld -nostdlib -Ttext=0x10200 -o loader64.elf loader64.o keyboard.o print.o driverp.o harddisk.o graphicscard.o
# 6. 提取.text段（现在包含两个模块的代码）
objcopy -O binary --only-section=.text loader64.elf loader64.bin

# 7. 验证提取的内容
echo "loader64.bin大小: $(stat -c%s loader64.bin) 字节"
hexdump -C loader64.bin | head -5

# 8. 写入磁盘（从扇区2开始）
dd if=loader64.bin of=hd.raw bs=1 seek=1024 conv=notrunc

# 9. 验证磁盘写入
echo "=== 磁盘验证 ==="
echo "扇区2开头（应该非零）:"
dd if=hd.raw bs=512 skip=2 count=1 2>/dev/null | hexdump -C | head -3

# 10. 生成符号表用于调试
echo "=== 符号地址 ==="
nm loader64.elf | grep -E "(keyboard_handler|keyboard_init|setup_keyboard_interrupt)"

# 在原脚本的 #9 或 #10 步骤之后，添加：

echo "=== 创建最终整合镜像 Xprensive0-OS.bin ==="

# 方法：按引导顺序直接拼接三个二进制文件
cat boot.bin loader.bin loader64.bin > Xprensive0-OS.bin

# 验证最终文件大小
BOOT_SIZE=$(stat -c%s boot.bin)
LOADER_SIZE=$(stat -c%s loader.bin)
loader64_SIZE=$(stat -c%s loader64.bin)
TOTAL_SIZE=$((BOOT_SIZE + LOADER_SIZE + loader64_SIZE))

echo "各组件大小:"
echo "  boot.bin:    $BOOT_SIZE 字节"
echo "  loader.bin:  $LOADER_SIZE 字节"
echo "  loader64.bin:  $loader64_SIZE 字节"
echo "  总大小:      $(stat -c%s Xprensive0-OS.bin) 字节 (应为 $TOTAL_SIZE)"

# 验证拼接是否正确（检查开头512字节是否与boot.bin一致）
echo "=== 验证镜像开头 ==="
head -c 512 Xprensive0-OS.bin | hexdump -C | head -3

echo "=== 完成! ==="
echo "最终镜像: Xprensive0-OS.bin"
echo "可以直接使用: qemu-system-x86_64 -loader64 Xprensive0-OS.bin"