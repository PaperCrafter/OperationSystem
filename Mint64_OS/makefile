all: BootLoader Kernel32 Kernel64 Disk.img Utility

BootLoader:
	@echo
	@echo ================= Build Boot Loader =================
	@echo
	
	make -C 00.BootLoader
	
	@echo
	@echo ================= Build Complete =================
	@echo
	
Kernel32:
	@echo
	@echo ================= Build 32bit Kernel =================
	@echo
	
	make -C 01.Kernel32
	
	@echo
	@echo ================= Build Complete =================
	@echo

Kernel64:
	@echo
	@echo ================= Build 64bit Kernel =================
	@echo 
	
	make -C 02.Kernel64
	
	@echo
	@echo ================= Build Complete =================
	@echo
	
Disk.img: 00.BootLoader/BootLoader.bin 01.Kernel32/Kernel32.bin 02.Kernel64/Kernel64.bin
	@echo
	@echo ================= Disk Image Build Start =================
	@echo
	
	./ImageMaker.exe $^
	
	@echo
	@echo ================= All Build Complete =================
	@echo
	
Utility:
	@echo
	@echo ================= Utility Build Start =================
	@echo

	@echo
	@echo ================= Utility Build Complete =================
	@echo
	
debug:
	qemu-system-x86_64 -s -singlestep -S -L . -fda Disk.img -m 64 -localtime -M pc

run:
	#qemu-system-x86_64 -L . -fda Disk.img -m 64 -localtime -M pc
	qemu-system-x86_64 -L . -m 64 -fda ./Disk.img -hda ./HDD.img -boot a -localtime -M pc

clean:
	make -C 00.BootLoader clean
	make -C 01.Kernel32 clean
	make -C 02.Kernel64 clean
	make -C 04.Utility clean
	rm -f Disk.img
