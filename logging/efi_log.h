#ifndef EFI_LOG_H
#define EFI_LOG_H

#include <wchar.h>

#include <efi.h>
#include <efilib.h>

void print_status(EFI_STATUS status);
void log_message(const wchar_t *component, const wchar_t *message);
void stop_boot(const wchar_t *error, EFI_STATUS status);

#endif /* EFI_LOG_H */

