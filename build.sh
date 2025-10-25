# 1. 创建空的虚拟硬盘文件
dd if=/dev/zero of=hd.raw bs=1M count=10

# 2. 编译引导程序
nasm -f bin boot.asm -o boot.bin
nasm -f bin loader.asm -o loader.bin
nasm -f bin loader64.asm -o loader64.bin

# 3. 直接写入虚拟硬盘（不是img软盘格式）
dd if=boot.bin of=hd.raw bs=512 count=1 conv=notrunc
dd if=loader.bin of=hd.raw bs=512 seek=1 conv=notrunc
dd if=loader64.bin of=hd.raw bs=512 seek=2 conv=notrunc

# 4. 验证写入
hexdump -C hd.raw | head -30