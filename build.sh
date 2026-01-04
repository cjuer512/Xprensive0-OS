#!/bin/bash
rm -f hd.raw *.bin *.elf *.o

# 1. 创建虚拟硬盘
dd if=/dev/zero of=hd.raw bs=512 count=6144 2>/dev/null

# 2. boot.asm (第一扇区, 0x7C00)
nasm -f bin -o boot.bin boot.asm
truncate -s 512 boot.bin
dd if=boot.bin of=hd.raw bs=512 count=1 conv=notrunc

# 3. loader.asm (第二扇区, 运行在0x10000)
nasm -f bin -o loader.bin loader.asm
truncate -s 512 loader.bin
dd if=loader.bin of=hd.raw bs=512 seek=1 conv=notrunc

# 4. loader64.c (第三扇区开始, 运行在0x10200)
# 关键修改：正确提取和写入
gcc -c -m64 -ffreestanding -nostdlib -fno-builtin \
    -mno-red-zone -mgeneral-regs-only \
    loader64.c -o loader64.o

ld -nostdlib -Ttext=0x10200 -o loader64.elf loader64.o

# 方法A：直接用objcopy提取.text段
objcopy -O binary --only-section=.text loader64.elf loader64.bin

# 方法B：或者用dd从ELF提取（如果你坚持）
# TEXT_OFFSET=0x500   # .text段在ELF文件中的偏移
# TEXT_SIZE=0x98      # .text段大小
# dd if=loader64.elf of=loader64.bin bs=1 skip=$TEXT_OFFSET count=$TEXT_SIZE

# 计算需要多少扇区
SIZE=$(stat -c%s loader64.bin 2>/dev/null || echo 0)
if [ $SIZE -eq 0 ]; then
    echo "错误：loader64.bin是空的或不存在"
    exit 1
fi

SECTORS=$(( ($SIZE + 511) / 512 ))
echo "loader64.bin: $SIZE 字节，需要 $SECTORS 个扇区"

# 写入磁盘（从扇区2开始）
dd if=loader64.bin of=hd.raw bs=512 seek=2 conv=notrunc

# 5. 验证写入
echo "验证写入："
dd if=hd.raw bs=512 skip=2 count=1 2>/dev/null | hexdump -C | head -5

echo "完成！"
echo "  扇区0: boot.asm"
echo "  扇区1: loader.asm" 
echo "  扇区2开始 ($SECTORS扇区): loader64.c"