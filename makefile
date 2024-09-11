CC := gcc
AS := nasm
BL_ASFLAGS := -f bin

all: bootloader
	cp bin/bootloader.bin image.bin
	qemu-img resize image.bin 512M
	qemu-system-i386 bin/bootloader.bin --no-reboot --no-shutdown -m 32m -smp 2

bootloader:
	$(AS) src/bootloader/main.s $(BL_ASFLAGS) -o bin/bootloader.bin
init:
	@mkdir mount
	@mkdir bin
	@mkdir bin/boot
	@mkdir bin/kernel

clean:
	@rm -rf bin/
	@rm -rf mount/