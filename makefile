CC := gcc
AS := nasm
BL_ASFLAGS := -f bin

all: bootloader tools
	cp bin/bootloader.bin image.bin
	qemu-img resize -f raw image.bin 512M
	./diskwrite -v kernel.elf -o image.bin 
	qemu-system-i386 -hda image.bin --no-reboot --no-shutdown -m 32m -smp 2 -serial mon:stdio
	
tools:
	$(CC) tools/diskwrite.c -o diskwrite

bootloader:
	$(AS) src/bootloader/main.s $(BL_ASFLAGS) -o bin/bootloader.bin
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