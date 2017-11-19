#include <stdbool.h>

#include <efi.h>
#include <efilib.h>

#include "serial.h"
#include "kernel/kernel.h"
#include "kernel/memory_map.h"

EFI_STATUS efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
   init_serial();
   
   InitializeLib(ImageHandle, SystemTable);
   
   // Clear screen
   uefi_call_wrapper(ST->ConOut->ClearScreen, 1, ST->ConOut);
   Print(L"Loading Oh-my OS...\n");
   
   kernel_params init_parameters;
   fill_memory_map(&init_parameters.efi_memory_map);
   
   for(;;);

   return EFI_SUCCESS;
}
