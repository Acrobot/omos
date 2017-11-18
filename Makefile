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
	
CFLAGS-serial.o	+= -mcmodel=large
CFLAGS		+= $(CFLAGS-$@) $(EFIINCS)

ifeq ($(ARCH),x86_64)
  CFLAGS += -DEFI_FUNCTION_WRAPPER
endif

LDFLAGS         = -nostdlib -znocombreloc -T $(EFI_LDS) -shared \
		  -Bsymbolic -L $(EFILIB) -L $(LIB) $(EFI_CRT_OBJS) 

all: disk.img
	
serial-loader.o: serial.c
	$(CC) $(CFLAGS) -fpic -c -o $@ $^

loader.so: bootloader.o serial-loader.o
	ld $(LDFLAGS) $^ -o $@ -lefi -lgnuefi

%.efi: %.so
	objcopy -j .text -j .sdata -j .data -j .dynamic \
		-j .dynsym  -j .rel -j .rela -j .reloc \
		--target=efi-app-$(ARCH) $^ $@

disk.img: loader.efi
	dd if=/dev/zero of=$@ bs=1k count=1440
	mformat -i $@ -f 1440 ::
	mmd -i $@ ::/EFI
	mmd -i $@ ::/EFI/BOOT
	mcopy -i $@ $^ ::/EFI/BOOT/BOOTx64.EFI

run-qemu: disk.img OVMF.fd
	./run.sh

clean:
	rm -f *.efi *.o *.so *.img
	rm -rf hda