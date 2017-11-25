#include <stdbool.h>

#include <efi.h>
#include <efilib.h>

#include "serial.h"
#include "logging/efi_log.h"
#include "kernel/kernel.h"
#include "kernel/memory_map.h"
#include "stdlib.h"

extern char _binary_kernel_img_start;
extern char _binary_kernel_img_end;

EFI_STATUS efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
    init_serial();

    InitializeLib(ImageHandle, SystemTable);

    // Clear screen
    ST->ConOut->ClearScreen(ST->ConOut);
    Print(L"Loading Oh-my OS...\r\n");

    kernel_params init_parameters;
    
    initialize_framebuffer(&init_parameters.framebuffer);
    Print(L"Initialized framebuffer!\r\n");
    
    fill_memory_map(&init_parameters.efi_memory_map);
    
    // Printing (or using any UEFI functions touching memory) will invalidate
    // the memory map key. Just don't.

    EFI_STATUS status = ST->BootServices->ExitBootServices(ImageHandle, 
            init_parameters.efi_memory_map.map_key);
    
    // Printing will crash the machine if ExitBootServices succeeded.

    if (status != EFI_SUCCESS) {
        print_status(status);

        return EFI_LOAD_ERROR;
    } else {
        serial_print("Successfully exited from boot services\r\n");
    }
    
    void *kernel_base = (void *) KERNEL_START;
    const void *kernel_img = (const void *) &_binary_kernel_img_start;
    size_t kernel_img_size = ((size_t) &_binary_kernel_img_end) - ((size_t) kernel_img);
    
    serial_print("Starting kernel\r\n");
    serial_print_ptr(kernel_base);
    serial_print_ptr(&init_parameters);
    
    memcpy(kernel_base, kernel_img, kernel_img_size);
    init_kernel start_function = (init_kernel) kernel_base;
    
    start_function(&init_parameters);

    return EFI_SUCCESS;
}
