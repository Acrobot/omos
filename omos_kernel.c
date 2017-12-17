#include "kernel/kernel.h"
#include "kernel/interrupts.h"
#include "serial.h"
#include "gfx/console.h"

#include <efi.h>
#include <efilib.h>

void print_time_int(int number) {
    char buffer[3];
    buffer[0] = (number / 10) + '0';
    buffer[1] = (number % 10) + '0';
    buffer[2] = '\0';
    
    console_print((const char *) &buffer);
}

void kernel_main(kernel_params *params) {
    serial_print("Hello from the other side!");
    
    console_init(&params->framebuffer);
    console_println("Loaded Oh-my OS!");
    console_println("This is a very long line that should be hopefully broken"
            " down into lines");
    
    EFI_TIME time;
    EFI_STATUS status = params->efi_runtime_services->GetTime(&time, NULL);
    
    if (status == EFI_SUCCESS) {
        console_print("The time is now: ");
        print_time_int(time.Hour);
        console_print(":");
        print_time_int(time.Minute);
        console_print(":");
        print_time_int(time.Second);
    } else {
        console_print("Getting time failed!");
    }
    
    enable_interrupts();
    
    for(;;);
}

