all: BootLoader.bin BootLoader2.bin BootLoader3.bin

BootLoader.bin: BootLoader.asm
	nasm -o BootLoader.bin BootLoader.asm

BootLoader2.bin: BootLoader2.asm
	nasm -o BootLoader2.bin BootLoader2.asm

BootLoader3.bin: BootLoader3.asm
	nasm -o BootLoader3.bin BootLoader3.asm

clean:
	rm -f BootLoader.bin
	rm -f BootLoader2.bin
	rm -f BootLoader3.bin