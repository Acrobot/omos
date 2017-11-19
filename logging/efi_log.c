#include "efi_log.h"

void print_status(EFI_STATUS status) {
    #define CASE(x) case x: Print(L"status = "  L ## #x "\r\n"); break;
    switch(status) {
        CASE(EFI_SUCCESS)
        CASE(EFI_BUFFER_TOO_SMALL)
        CASE(EFI_OUT_OF_RESOURCES)
        CASE(EFI_INVALID_PARAMETER)
        CASE(EFI_NOT_FOUND)
        default: Print(L"status = <unknown>\r\n");
    }
    #undef CASE
}

void log(const wchar_t *component, const wchar_t *message) {
    Print(L"%s: %s\r\n", component, message);
}

void stop_boot(const wchar_t *error, EFI_STATUS status) {
    Print(L"error: %s\r\n", error);
    print_status(status);
    
    for (;;); // Infinite loop
}
