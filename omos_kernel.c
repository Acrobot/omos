#include "kernel/kernel.h"
#include "serial.h"
#include "gfx/console.h"

void kernel_main(kernel_params *params) {
    serial_print("Hello from the other side!");
    
    console_init(&params->framebuffer);
    console_print("asdasdasd");
    
    for(;;);
}

