CC := gcc
AS := nasm
CFLAGS := -c -mno-sse -mno-sse2 -mno-red-zone -ffreestanding -fno-pie -fno-stack-protector -mno-mmx
BL_ASFLAGS := -f bin
SRCS := $(wildcard src/kernel/*.c)
OBJS := $(patsubst src/kernel/%.c, bin/kernel/%.o, $(SRCS))

all: bootloader tools kernel
	cp bin/bootloader.bin image.bin
	qemu-img resize -f raw image.bin 512M
	./diskwrite -v kernel.elf -o image.bin
	qemu-system-i386 -hda image.bin --no-reboot --no-shutdown -m 32m -smp 2 -serial mon:stdio
	
tools:
	$(CC) tools/diskwrite.c -o diskwrite

bootloader:
	$(AS) src/bootloader/main.s $(BL_ASFLAGS) -o bin/bootloader.bin

kernel: $(OBJS)
	nasm src/kernel/entry.s -o bin/kernel/entry.o -f elf32
	ld -T linker.ld bin/kernel/entry.o -melf_i386

%.o: $(SRCS)
	$(CC) $(CFLAGS) $< -o $@

init:
	@mkdir mount
	@mkdir bin
	@mkdir bin/boot
	@mkdir bin/kernel

.PHONY: tools

clean:
	@rm diskwrite
	@rm image.bin
	@rm -rf bin/
	@rm -rf mount/