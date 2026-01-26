# ========== 操作系统构建Makefile ==========
# 基于你的shell脚本转换

# 1. 工具配置
NASM    := nasm
CC      := gcc
LD      := ld
OBJCOPY := objcopy
DD      := dd
TRUNCATE:= truncate
NM      := nm

# 2. 目标文件定义
HD_IMAGE    := hd.raw
BOOT_BIN    := temp/boot.bin
LOADER_BIN  := temp/loader.bin
LOADER64_ELF:= xprensive-debug.elf
LOADER64_BIN:= loader64.bin
SYM_FILE    := xprensive-debug.sym

# 3. 源文件和目录
BOOT_SRC    := bootloader/boot.asm
LOADER_SRC  := bootloader/loader.asm
LINKER_SCRIPT := link64.ld

# C源文件列表（按你的脚本顺序）
C_SOURCES := \
    bootloader/loader64.c \
    driver/keyboard.c \
    driver/driverp.c \
    driver/harddisk.c \
    driver/graphicscard.c \
    kernel/api/print.c \
    kernel/init.c \
    kernel/sucmd.c \
    kernel/set/syscall.c \
    kernel/set/user.c

# 生成对应的.o文件列表
C_OBJECTS := $(C_SOURCES:%.c=temp/%.o)

# 4. 编译标志
NASM_FLAGS    := -f bin
CC_FLAGS      := -c -m64 -ffreestanding -nostdlib -fno-builtin \
                 -mno-red-zone -mgeneral-regs-only -g
LD_FLAGS      := -nostdlib -T $(LINKER_SCRIPT) -g
OBJCOPY_FLAGS := -O binary
DD_FLAGS      := bs=512 conv=notrunc

# init.c和sucmd.c需要-O0优化
CC_FLAGS_O0   := $(CC_FLAGS) -O0

# 5. 默认目标（构建完整系统）
.PHONY: all
all: $(HD_IMAGE) $(SYM_FILE)
	@echo "=== 构建完成 ==="
	@echo "硬盘镜像: $(HD_IMAGE)"
	@echo "符号文件: $(SYM_FILE)"

# 6. 清理目标
.PHONY: clean
clean:
	rm -f hd.raw *.bin *.elf *.o *.gdb *.map *.log *.s xprensive-debug.sym
	rm -rf temp

# 7. 创建临时目录
temp:
	mkdir -p temp

# 8. 构建虚拟硬盘镜像
$(HD_IMAGE): temp $(BOOT_BIN) $(LOADER_BIN) $(LOADER64_BIN)
	@echo "=== 创建虚拟硬盘 ==="
	$(DD) if=/dev/zero of=$@ bs=512 count=16384 2>/dev/null
	
	@echo "写入 boot.bin (第一扇区, 0x7C00)"
	$(DD) if=$(BOOT_BIN) of=$@ $(DD_FLAGS) count=1
	@echo "boot.bin写入状态: $$?"
	
	@echo "写入 loader.bin (第二扇区, 0x10000)"
	$(DD) if=$(LOADER_BIN) of=$@ $(DD_FLAGS) seek=1
	@echo "loader.bin写入状态: $$?"
	
	@echo "写入 loader64.bin (从扇区2开始)"
	$(DD) if=$(LOADER64_BIN) of=$@ $(DD_FLAGS) seek=2

# 9. 编译引导扇区
$(BOOT_BIN): $(BOOT_SRC) | temp
	@echo "编译 boot.asm"
	$(NASM) $(NASM_FLAGS) -o $@ $<
	$(TRUNCATE) -s 512 $@

# 10. 编译loader
$(LOADER_BIN): $(LOADER_SRC) | temp
	@echo "编译 loader.asm"
	$(NASM) $(NASM_FLAGS) -o $@ $<
	$(TRUNCATE) -s 512 $@

# 11. 编译C文件规则
# 通用规则（大多数文件）
temp/%.o: %.c | temp
	@mkdir -p $(@D)
	@echo "编译 $<"
	$(CC) $(CC_FLAGS) $< -o $@

# init.c和sucmd.c需要-O0
temp/kernel/init.o: kernel/init.c | temp
	@mkdir -p $(@D)
	@echo "编译 $< (带-O0)"
	$(CC) $(CC_FLAGS_O0) $< -o $@

temp/kernel/sucmd.o: kernel/sucmd.c | temp
	@mkdir -p $(@D)
	@echo "编译 $< (带-O0)"
	$(CC) $(CC_FLAGS_O0) $< -o $@

# 12. 链接ELF文件
$(LOADER64_ELF): $(C_OBJECTS) $(LINKER_SCRIPT)
	@echo "=== 链接生成调试ELF ==="
	$(LD) $(LD_FLAGS) -o $@ $(C_OBJECTS)

# 13. 从ELF提取二进制
$(LOADER64_BIN): $(LOADER64_ELF)
	@echo "提取二进制文件"
	$(OBJCOPY) $(OBJCOPY_FLAGS) $< $@

# 14. 生成符号表
$(SYM_FILE): $(LOADER64_ELF)
	@echo "生成符号表"
	$(NM) $< > $@

# 15. 辅助目标
.PHONY: symbols
symbols: $(SYM_FILE)
	@echo "符号表已生成: $(SYM_FILE)"

.PHONY: elf
elf: $(LOADER64_ELF)
	@echo "ELF文件已生成: $(LOADER64_ELF)"

.PHONY: bin
bin: $(LOADER64_BIN)
	@echo "二进制文件已生成: $(LOADER64_BIN)"

.PHONY: boot-only
boot-only: $(BOOT_BIN) $(LOADER_BIN)
	@echo "仅编译引导程序完成"

# 16. 运行目标（假设用QEMU）
QEMU := qemu-system-x86_64
QEMU_FLAGS := -drive file=$(HD_IMAGE),format=raw -m 512M

.PHONY: run
run: all
	@echo "=== 在QEMU中启动 ==="
	$(QEMU) $(QEMU_FLAGS)

.PHONY: debug
debug: all
	@echo "=== 在QEMU中调试启动 ==="
	@echo "在另一个终端运行: gdb -ex 'target remote localhost:1234' -ex 'symbol-file $(LOADER64_ELF)'"
	$(QEMU) $(QEMU_FLAGS) -s -S

# 17. 查看帮助
.PHONY: help
help:
	@echo "可用目标:"
	@echo "  all          - 构建完整系统 (默认)"
	@echo "  clean        - 清理所有生成文件"
	@echo "  boot-only    - 仅编译引导程序"
	@echo "  elf          - 仅生成ELF文件"
	@echo "  bin          - 仅生成二进制文件"
	@echo "  symbols      - 仅生成符号表"
	@echo "  run          - 在QEMU中运行系统"
	@echo "  debug        - 以调试模式启动QEMU"
	@echo "  help         - 显示此帮助信息"
	@echo ""
	@echo "重要文件:"
	@echo "  $(HD_IMAGE)     - 虚拟硬盘镜像"
	@echo "  $(LOADER64_ELF) - 带调试信息的ELF文件"
	@echo "  $(SYM_FILE)     - 符号表文件"

# 18. 依赖关系声明
$(C_OBJECTS): | temp

# 19. 包含自动生成的依赖（可选）
# -include $(C_OBJECTS:.o=.d)