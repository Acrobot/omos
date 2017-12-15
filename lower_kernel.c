#include "kernel/kernel.h"
#include "kernel/paging.h"
#include "serial.h"

extern char _binary_omos_kernel_img_start;

static void start_kernel(const kernel_params *params) {
    init_kernel start = (init_kernel) KERNEL_VIRTUAL_BASE;
    SERIAL_DUMP_HEX(start);
    start(params);
}

void start_omos_kernel(const kernel_params *params) {
    serial_print("Hello, lower kernel\r\n");
    serial_print("Loading paging...\r\n");
    
    char *kernel_img = (char *) &_binary_omos_kernel_img_start;
    
    kernel_params new_params = *params;
    
    enable_paging(kernel_img, &new_params.efi_memory_map, &new_params.framebuffer);
    serial_print("Paging enabled\r\n");
    
    new_params.framebuffer.base = (void *) FRAMEBUFFER_START;
    
    serial_print("Starting new kernel\r\n");
    start_kernel(&new_params);
}
