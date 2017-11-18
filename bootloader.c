#include <stdbool.h>

#include <efi.h>
#include <efilib.h>

#include "serial.h"

EFI_STATUS efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
   init_serial();
   
   InitializeLib(ImageHandle, SystemTable);
   
   // Clear screen
   uefi_call_wrapper(ST->ConOut->ClearScreen, 1, ST->ConOut);
   Print(L"Hello, world!\n");
   
   for(;;);

   return EFI_SUCCESS;
}
