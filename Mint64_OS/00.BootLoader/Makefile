all: BootLoader1.bin BootLoader2.bin BootLoader.bin

BootLoader1.bin: BootLoader1.asm
	nasm -o BootLoader1.bin BootLoader1.asm

BootLoader2.bin: BootLoader2.asm
	nasm -o BootLoader2.bin BootLoader2.asm

BootLoader.bin:BootLoader1.bin BootLoader2.bin
	cat $^ > BootLoader.bin

clean:
	rm -f BootLoader1.bin
	rm -f BootLoader2.bin
	rm -f BootLoader.bin