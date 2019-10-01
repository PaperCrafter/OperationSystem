all: BootLoader.bin BootLoader2.bin

BootLoader.bin: BootLoader.asm
	nasm -o BootLoader.bin BootLoader.asm

BootLoader2.bin: BootLoader2.asm
	nasm -o BootLoader2.bin BootLoader2.asm

clean:
	rm -f BootLoader.bin
	rm -f BootLoader2.bin
