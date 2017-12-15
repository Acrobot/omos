#ifndef PAGING_H
#define PAGING_H

#include <stddef.h>
#include "memory_map.h"

#define PAGE_SIZE 4096  // 4 KiB
#define ADDRESS_BITS 12
#define PAGE_ALIGNED __attribute__((aligned(PAGE_SIZE)))
#define PACKED __attribute__((packed))

typedef struct {
	uint16_t offset: 12;
	uint16_t pt: 9;
	uint16_t pd: 9;
	uint16_t pdpt: 9;
	uint16_t pml4: 9;
	uint16_t reserved: 16;
} PACKED linear_address;

_Static_assert(sizeof(linear_address) == sizeof(uint64_t), "linear address has to have 64 bits");

void enable_paging(EFI_MEMORY_MAP *memory_map, framebuffer *fb);

#endif /* PAGING_H */

