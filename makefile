CC := gcc
AS := nasm
CFLAGS = -c -mno-sse -mno-sse2 -mno-red-zone -ffreestanding -m32 -nostdlib -O3 -fno-pie -fno-stack-protector -mno-mmx
BL_ASFLAGS := -f bin
SRCS := $(wildcard src/kernel/*/*.c)
OBJS := $(patsubst src/kernel/%.c, bin/kernel/%.o, $(SRCS))

all: bootloader tools kernel
	cp bin/bootloader.bin image.bin
	qemu-img resize -f raw image.bin 512M
	./diskwrite -v kernel.elf -o image.bin
	qemu-system-i386 -hda image.bin --no-reboot --no-shutdown -m 32m -smp 2 -serial mon:stdio -D intlog.txt -d int
tools:
	$(CC) tools/diskwrite.c -o diskwrite

bootloader:
	$(AS) src/bootloader/main.s $(BL_ASFLAGS) -o bin/bootloader.bin

# kernel: $(OBJS)
kernel:
	nasm src/kernel/entry.s -o bin/kernel/entry.o -f elf32
	sh c_build_helper.sh
	# ld -T linker.ld bin/kernel/entry.o bin/kernel/*.o -melf_i386
	ld -T linker.ld bin/kernel/*.o -melf_i386
# %.o: $(SRCS)
# 	mkdir -p bin/kernel/$(shell dirname $@)
# 	$(CC) $(CFLAGS) $< -o $@

init:
	@mkdir mount
	@mkdir bin
	@mkdir bin/boot
	@mkdir bin/kernel

.PHONY: tools makefile src/

clean:
	@rm diskwrite
	@rm image.bin
	@rm -rf bin/
	@rm -rf mount/