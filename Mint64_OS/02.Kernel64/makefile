NASM64 = nasm -f elf64
GCC64 = gcc -c -m64 -ffreestanding
LD64 = gcc -m64 -T ../elf_x86_64.x -nostdlib -e Main -Ttext 0x200000

OBJCOPY64 = objcopy -j .text -j .data -j .rodata -j .bss -S -O binary

OBJECTDIRECTORY = Temp
SOURCEDIRECTORY	= Source


all: prepare Kernel64.bin


prepare:
	mkdir -p $(OBJECTDIRECTORY)

dep:
	@echo === Make Dependancy File ===
	make -C $(OBJECTDIRECTORY) -f ../makefile InternalDependency
	@echo === Dependancy Search Complete ===

# ���͸��� ������Ʈ ���� ���͸��� �̵��ؼ� ������ ���� �� ���� ������ ����
ExecuteInternalBuild: dep
	make -C $(OBJECTDIRECTORY) -f ../makefile Kernel64.elf

# Ŀ�� �̹����� ���̳ʸ� ���Ϸ� ��ȯ�Ͽ� IA-32e ��� ���̳ʸ� ����
Kernel64.bin: ExecuteInternalBuild
	$(OBJCOPY64) $(OBJECTDIRECTORY)/Kernel64.elf $@
		
# �ҽ� ������ ������ ������ ���� ����	
clean:
	rm -f *.bin
	rm -f $(OBJECTDIRECTORY)/*.*

################################################################################
# Make�� ���� �ٽ� ȣ��Ǵ� �κ�, Temp ���͸��� �������� �����
################################################################################
# ������ ������� ��Ʈ�� ����Ʈ �ҽ� ���� ����, Temp ���͸��� �������� ����
ENTRYPOINTSOURCEFILE = ../$(SOURCEDIRECTORY)/EntryPoint.s
ENTRYPOINTOBJECTFILE = EntryPoint.o
# ������ C �ҽ� ���� ����, Temp ���͸��� �������� ����
CSOURCEFILES = $(wildcard ../$(SOURCEDIRECTORY)/*.c)
ASSEMBLYSOURCEFILES = $(wildcard ../$(SOURCEDIRECTORY)/*.asm)
COBJECTFILES = $(notdir $(patsubst %.c,%.o,$(CSOURCEFILES)))
ASSEMBLYOBJECTFILES = $(notdir $(patsubst %.asm,%.o,$(ASSEMBLYSOURCEFILES)))

# ������� ��Ʈ�� ����Ʈ ����
$(ENTRYPOINTOBJECTFILE): $(ENTRYPOINTSOURCEFILE)
	$(NASM64) -o $@ $<

# .c ������ .o ���Ϸ� �ٲٴ� ��Ģ ����
%.o: ../$(SOURCEDIRECTORY)/%.c
	$(GCC64) -c $<

# .asm ������ .o ���Ϸ� �ٲٴ� ��Ģ ����
%.o: ../$(SOURCEDIRECTORY)/%.asm
	$(NASM64) -o $@ $<

# ���� �������� ���õ� ������ ����
InternalDependency:
	$(GCC64) -MM $(CSOURCEFILES) > Dependency.dep

# ���� Ŀ�� �̹����� ����
Kernel64.elf: $(ENTRYPOINTOBJECTFILE) $(COBJECTFILES) $(ASSEMBLYOBJECTFILES)
	$(LD64) -o $@ $^

# ���� ���丮�� ���� ��, dependency ������ ������ make�� ����	
ifeq (Dependency.dep, $(wildcard Dependency.dep))
include Dependency.dep
endif
