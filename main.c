#include <efi.h>
#include <efilib.h>

#include <stdbool.h>

EFI_STATUS efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
   InitializeLib(ImageHandle, SystemTable);
   
   // Clear screen
   uefi_call_wrapper(ST->ConOut->ClearScreen, 1, ST->ConOut);
   Print(L"Hello, world!\n");
   
   for(;;);

   return EFI_SUCCESS;
}
