CC := gcc
AS := nasm
CFLAGS:=-g -c -m32 -fno-pie -mno-sse -O3 -D __bits__=32 -Wno-int-to-pointer-cast -Wno-pointer-to-int-cast -Wno-incompatible-pointer-types -Wno-address-of-packed-member -Wno-discarded-qualifiers -fno-stack-protector -mno-red-zone -mno-sse -mno-sse2 -ffreestanding -nostdlib -mno-mmx
BL_ASFLAGS := -f bin
SRCS := $(wildcard src/kernel/*/*.c)
OBJS := $(patsubst src/kernel/%.c, bin/kernel/%.o, $(SRCS))

all: bootloader tools kernel
	cp bin/bootloader.bin image.bin
	qemu-img resize -f raw image.bin 512M
	./diskwrite -v kernel.elf -o image.bin
	./diskwrite -v idm.elf -o image.bin
	qemu-system-i386 -hda image.bin --no-reboot --no-shutdown -m 32m -smp 2 -serial mon:stdio -D intlog.txt -d int
tools:
	$(CC) tools/diskwrite.c -o diskwrite

bootloader:
	$(AS) src/bootloader/main.s $(BL_ASFLAGS) -o bin/bootloader.bin

# kernel: $(OBJS)
kernel:
	nasm src/kernel/entry.s -o bin/kernel/entry.o -f elf32
	gcc src/kmodules/disk_driver.c $(CFLAGS) -o bin/modules/disk_driver.o -m32
	sh c_build_helper.sh
	nasm src/kernel/arch_i386/idt.s -o bin/kernel/idt.o -felf32
	# ld -T linker.ld bin/kernel/entry.o bin/kernel/*.o -melf_i386
	ld -T linker.ld bin/kernel/*.o -melf_i386 -o kernel.elf
	ld -T linker.ld -o kernel_interface.elf -r -R kernel.elf -melf_i386
	ld bin/modules/disk_driver.o kernel_interface.elf -o idm.elf -melf_i386
# %.o: $(SRCS)
# 	mkdir -p bin/kernel/$(shell dirname $@)
# 	$(CC) $(CFLAGS) $< -o $@

init:
	mkdir mount
	mkdir bin
	mkdir bin/boot
	mkdir bin/kernel
	mkdir bin/modules

.PHONY: tools makefile src/

clean:
	rm diskwrite
	rm image.bin
	rm -rf bin/
	rm -rf mount/
	rm *.elf
	rm intlog.txt