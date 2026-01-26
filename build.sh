#!/bin/bash

# 只编译生成调试文件
echo "=== 编译带调试信息的文件 ==="
rm -f hd.raw *.bin *.elf *.o *.gdb *.map *.log *.s

# 1. 创建虚拟硬盘
dd if=/dev/zero of=hd.raw bs=512 count=16384 2>/dev/null




# 2. boot.asm (第一扇区, 0x7C00)
nasm -f bin -o temp/boot.bin bootloader/boot.asm
truncate -s 512 temp/boot.bin
dd if=temp/boot.bin of=hd.raw bs=512 count=1 conv=notrunc
echo "boot.bin写入状态: $?"
# 3. loader.asm (第二扇区, 运行在0x10000)
nasm -f bin -o temp/loader.bin bootloader/loader.asm
truncate -s 512 temp/loader.bin
dd if=temp/loader.bin of=hd.raw bs=512 seek=1 conv=notrunc  
echo "temp/loader.bin写入状态: $?"
# 编译每个C文件，生成带调试信息的.o文件
gcc -c -m64 -ffreestanding -nostdlib -fno-builtin \
    -mno-red-zone -mgeneral-regs-only \
    -g \
    bootloader/loader64.c -o temp/loader64.o

gcc -c -m64 -ffreestanding -nostdlib -fno-builtin \
    -mno-red-zone -mgeneral-regs-only \
    -g \
    driver/keyboard.c -o temp/keyboard.o


gcc -c -m64 -ffreestanding -nostdlib -fno-builtin \
    -mno-red-zone -mgeneral-regs-only \
    -g \
    driver/driverp.c -o temp/driverp.o

gcc -c -m64 -ffreestanding -nostdlib -fno-builtin \
    -mno-red-zone -mgeneral-regs-only \
    -g \
    driver/harddisk.c -o temp/harddisk.o

gcc -c -m64 -ffreestanding -nostdlib -fno-builtin \
    -mno-red-zone -mgeneral-regs-only \
    -g \
    driver/graphicscard.c -o temp/graphicscard.o
gcc -c -m64 -ffreestanding -nostdlib -fno-builtin \
    -mno-red-zone -mgeneral-regs-only \
    -g \
    kernel/api/print.c -o temp/print.o

gcc -c -m64 -ffreestanding -nostdlib -fno-builtin \
    -mno-red-zone -mgeneral-regs-only \
    -g -O0\
    kernel/init.c -o temp/init.o

gcc -c -m64 -ffreestanding -nostdlib -fno-builtin \
    -mno-red-zone -mgeneral-regs-only \
    -g -O0\
    kernel/sucmd.c -o temp/sucmd.o
gcc -c -m64 -ffreestanding -nostdlib -fno-builtin \
    -mno-red-zone -mgeneral-regs-only \
    -g -O0\
    kernel/set/syscall.c -o temp/syscall.o

gcc -c -m64 -ffreestanding -nostdlib -fno-builtin \
    -mno-red-zone -mgeneral-regs-only \
    -g -O0\
    kernel/set/user.c -o temp/user.o
# 链接生成带完整调试信息的ELF文件（去掉不支持的-gdwarf-2）
echo "=== 链接生成调试ELF ==="
echo "=== 链接生成调试ELF ==="
ld -nostdlib -T link64.ld \
    -g \
    -o xprensive-debug.elf \
    temp/loader64.o \
    temp/keyboard.o \
    temp/driverp.o \
    temp/harddisk.o \
    temp/graphicscard.o \
    temp/print.o \
    temp/init.o \
    temp/sucmd.o \
    temp/syscall.o \
    temp/user.o
objcopy -O binary xprensive-debug.elf loader64.bin 


# 8. 写入磁盘（从扇区2开始）
dd if=loader64.bin of=hd.raw bs=512 seek=2 conv=notrunc
# 生成符号表
if [ -f "xprensive-debug.elf" ]; then
    nm xprensive-debug.elf > xprensive-debug.sym
else
    echo "错误：ELF文件未生成"
    exit 1
fi

echo "=== 完成 ==="