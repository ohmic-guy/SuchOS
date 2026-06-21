ASM  = nasm
QEMU = qemu-system-i386

all: floppy.img

boot.bin: boot/boot.asm
	$(ASM) -f bin $< -o $@

stage2.bin: boot/stage2.asm
	$(ASM) -f bin $< -o $@

floppy.img: boot.bin stage2.bin
	dd if=/dev/zero  bs=512 count=2880 of=$@
	dd if=boot.bin   of=$@ conv=notrunc
	dd if=stage2.bin of=$@ bs=512 seek=1 conv=notrunc

run: floppy.img
	$(QEMU) -drive format=raw,file=floppy.img,if=floppy,index=0

clean:
	rm -f *.bin floppy.img
