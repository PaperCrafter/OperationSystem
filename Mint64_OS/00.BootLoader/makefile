Target = BootLoader.bin
ASM = BootLoader.asm

all: $(Target)

$(Target): $(ASM)
	nasm -o $(Target) $(ASM)

clean:
	rm -f $(Target)


