#include "memory_map.h"
#include "../logging/efi_log.h"

#define COMPONENT_NAME L"Memory map"

void fill_memory_map(EFI_MEMORY_MAP *efi_memory_map) {
    efi_memory_map->memory_map = (EFI_MEMORY_DESCRIPTOR *) memory_map_buffer;
    efi_memory_map->memory_map_size = MEMORY_MAP_BUFFER_SIZE;
    
    log_message(COMPONENT_NAME, L"Started retrieval...");
    
    EFI_STATUS status = ST->BootServices->GetMemoryMap(
        &efi_memory_map->memory_map_size,
        efi_memory_map->memory_map,
        &efi_memory_map->map_key,
        &efi_memory_map->descriptor_size,
        &efi_memory_map->descriptor_version);
    
    if (status != EFI_SUCCESS) {
        stop_boot(L"fill_memory_map failed", status);
    } else {
        log_message(COMPONENT_NAME, L"Retrieval successful");
    }
}

#undef COMPONENT_NAME
