ARCH            = $(shell uname -m | sed s,i[3456789]86,ia32,)

TARGET          = loader.efi

EFIINC          = /usr/include/efi
EFIINCS         = -I$(EFIINC) -I$(EFIINC)/$(ARCH) -I$(EFIINC)/protocol
LIB             = /usr/lib
EFILIB          = /usr/lib
EFI_CRT_OBJS    = $(EFILIB)/crt0-efi-$(ARCH).o
EFI_LDS         = $(EFILIB)/elf_$(ARCH)_efi.lds

CC		= /usr/bin/clang
CFLAGS          = -O0 -xc -std=gnu11 -fno-stack-protector -fshort-wchar -mno-red-zone -Wall -Wextra -pedantic-errors \
		  -DGNU_EFI_USE_MS_ABI -DGNU_EFI_USE_EXTERNAL_STDARG -fPIC
CFLAGS-lower_kernel.o		+= -mcmodel=large
CFLAGS		+= $(CFLAGS-$@) $(EFIINCS)

ifeq ($(ARCH),x86_64)
	CFLAGS += -DEFI_FUNCTION_WRAPPER
endif

LDFLAGS         = -nostdlib -znocombreloc -T $(EFI_LDS) -shared \
		  -Bsymbolic -L $(EFILIB) -L $(LIB) $(EFI_CRT_OBJS) 

all: disk.img
	
fontppm.o: gfx/font.ppm
	objcopy -I binary -O elf64-x86-64 -B i386 $^ $@
	
serial-loader.o: serial.c
	$(CC) $(CFLAGS) -fpic -c -o $@ $^

loader.so: bootloader.o stdlib.o serial-loader.o kernel/memory_map.o logging/efi_log.o lower_kernelimg.o
	ld $(LDFLAGS) $^ -o $@ -lefi -lgnuefi
	
omos_kernel.elf.img: omos_krnl.o omos_kernel.o serial.o kernel/paging.o gfx/console.o fontppm.o
	ld -T kernel/kernel.ld -o $@ $^

omos_kernel.img: omos_kernel.elf.img
	objcopy -O binary $^ $@
	
omos_kernelimg.o: omos_kernel.img
	objcopy -I binary -O elf64-x86-64 -B i386 $^ $@
	
lower_kernel.elf.img: lower_krnl.o lower_kernel.o serial.o kernel/paging.o omos_kernelimg.o
	ld -T lower_kernel.ld -o $@ $^

lower_kernel.img: lower_kernel.elf.img
	objcopy -O binary $^ $@
	
lower_kernelimg.o: lower_kernel.img
	objcopy -I binary -O elf64-x86-64 -B i386 $^ $@

%.efi: %.so
	objcopy -j .text -j .sdata -j .data -j .dynamic \
		-j .dynsym  -j .rel -j .rela -j .reloc \
		--target=efi-app-$(ARCH) $^ $@

disk.img: loader.efi
	dd if=/dev/zero of=$@ bs=1k count=2880
	mformat -i $@ -f 2880 ::
	mmd -i $@ ::/EFI
	mmd -i $@ ::/EFI/BOOT
	mcopy -i $@ $^ ::/EFI/BOOT/BOOTx64.EFI

run-qemu: disk.img OVMF.fd
	./run.sh

clean:
	find . \( -name '*.efi' -o -name '*.o' -o -name '*.so' \
	    -o -name '*.img' \) -delete