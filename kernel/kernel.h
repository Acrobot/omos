#ifndef KERNEL_H
#define KERNEL_H

#include "memory_map.h"

/* Start of kernel sections in memory, synced with lower_kernel.ld */
#define KERNEL_START (1 * 1024 * 1024)

#define KERNEL_VIRTUAL_BASE 0xffff800000000000 // sync with omos_kernel.ld
#define EFI_VIRTUAL_BASE (KERNEL_VIRTUAL_BASE + 0x40000000)
#define FRAMEBUFFER_START (EFI_VIRTUAL_BASE + 0x40000000)

typedef struct {
    const EFI_HANDLE image_handle;
    EFI_MEMORY_MAP efi_memory_map;
    EFI_RUNTIME_SERVICES *efi_runtime_services;
    framebuffer framebuffer;
} kernel_params;

typedef void (*init_kernel)(const kernel_params *);

#endif /* KERNEL_H */

