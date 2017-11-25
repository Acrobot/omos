#include "kernel/kernel.h"
#include "serial.h"

void kernel_main(kernel_params *params) {
    serial_print("Hello from the other side!");
    
    
    for(;;);
}

