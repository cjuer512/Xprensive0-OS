gcc -c -m64 -ffreestanding -nostdlib -fno-builtin \
    -mno-red-zone -mgeneral-regs-only \
    -g -O0\
    kernel/sucmd.c -o temp/sucmd.o

gcc -c -m64 -ffreestanding -nostdlib -fno-builtin \
    -mno-red-zone -mgeneral-regs-only \
    -g \
    kernel/suapi/print.c -o temp/print.o

ld -nostdlib -T kernel/link_kernel.ld \
    -g \
    -o xprensive-kernel.elf \
    temp/sucmd.o \
    temp/print.o \
    temp/driverp.o \
    temp/keyboard.o
objcopy -O binary xprensive-kernel.elf kernel.bin 
tools/linktool kernel.bin kernelcmd.bin rwea