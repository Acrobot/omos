qemu-system-x86_64 -monitor stdio -net none -pflash ./OVMF.fd -drive file=disk.img,format=raw -s -no-reboot
