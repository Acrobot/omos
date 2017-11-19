#ifndef MEMORY_MAP_H
#define MEMORY_MAP_H

#include <efi.h>
#include <efilib.h>

#define MEMORY_MAP_BUFFER_SIZE 512 * 1024 // 512 KiB
char memory_map_buffer[MEMORY_MAP_BUFFER_SIZE];

typedef struct {
	EFI_MEMORY_DESCRIPTOR *memory_map;
	UINT64 memory_map_size;
	UINT64 map_key;
	UINT64 descriptor_size;
	UINT32 descriptor_version;
} EFI_MEMORY_MAP;

void fill_memory_map(EFI_MEMORY_MAP *efi_memory_map);

#endif /* MEMORY_MAP_H */

