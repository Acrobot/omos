#include <efi.h>
#include <efilib.h>

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
    }
}

void initialize_framebuffer(framebuffer *fb) {
    EFI_GUID gop_guid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
    EFI_GRAPHICS_OUTPUT_PROTOCOL *gop;
    
    ST->BootServices->LocateProtocol(&gop_guid, NULL, (void **) &gop);
    
    fb->base = (void *) gop->Mode->FrameBufferBase;
    fb->size = gop->Mode->FrameBufferSize;
    
    fb->width = gop->Mode->Info->HorizontalResolution;
    fb->height = gop->Mode->Info->VerticalResolution;
    fb->pixelsPerScanline = gop->Mode->Info->PixelsPerScanLine;
    
    gop->SetMode(gop, gop->Mode->Mode);
}

#undef COMPONENT_NAME
