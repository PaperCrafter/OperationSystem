all: BootLoader1.bin BootLoader2.bin BootLoader.bin

BootLoader1.bin: BootLoader1.asm
	nasm -o BootLoader1.bin BootLoader1.asm

BootLoader2.bin: BootLoader2.asm
	nasm -o BootLoader2.bin BootLoader2.asm

BootLoader.bin: 
	cat BootLoader1.bin BootLoader2.bin > BootLoader.bin
	
clean:
	rm -f BootLoader.bin
	
