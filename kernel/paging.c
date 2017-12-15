#include "paging.h"
#include "../serial.h"
#include "kernel.h"

#include <efi.h>

/* 512 entries * 8 bytes = 4 KiB
 * PML4 size = PDP size = PD size = PT size 
 * 
 * Since we are using 64 bit mode, the entry size
 * is 64 bits.
 */
#define PAGE_TABLE_SIZE 512

/*
 * How many pages to allocate in order to assign
 * them later to be used by paging?
 */
#define PAGES_TO_ALLOCATE 32

/* Entry in a page table */
typedef struct {
    /* Is the page present in physical memory? */
    uint8_t present : 1;
    
    /* Pages are read-only by default */
    uint8_t write_allowed : 1;
    
    /* Pages are only accessible by supervisor by default */
    uint8_t accessible_by_all : 1;
    
    /* Write through abilities of the page */
    uint8_t write_through : 1;
    
    /* If set, the page will not be cached */
    uint8_t cache_disabled : 1;
    
    /* Was the page accessed?*/
    uint8_t accessed : 1;
    
    /* 
     * Has the page been written to?
     * Only applicable for PTE.
     */
    uint8_t dirty : 1;
    
    /* 
     * Page size by default is small,
     * enabling this bit makes it bigger.
     * Only applicable for >PTE 
     */
    uint8_t large_page : 1;
    
    /* 
     * Prevent the translations cache from updating
     * the address in cache if CR3 is reset.
     * 
     * Page global enable bit in CR4 has to be set to
     * enable this feature.
     * 
     * Only applicable for PTE 
     */
    uint8_t global : 1;
    
    /* Not used by the processor */
    uint8_t metadata : 3;
    
    /* Physical address of the child table/page */
    uint64_t address  : 40;
    
    /* Not used by the processor */
    uint8_t metadata_2 : 7;
    
    /* Only applicable for PTE */
    uint8_t protection_key : 4;
   
    /* Disable execution of code from this page */
    uint8_t disable_execution : 1;
} PACKED page_entry;

_Static_assert(sizeof(page_entry) == sizeof(uint64_t), "page entry has to be 64 bits");

page_entry pml4[PAGE_TABLE_SIZE] PAGE_ALIGNED;

static inline void write_cr3(uint64_t value) {
    __asm__("movq %0, %%cr3" :: "r"(value));
}

static uint8_t pages[PAGES_TO_ALLOCATE][PAGE_SIZE] PAGE_ALIGNED;
static int last_page_index = 0;

static inline void *allocate_page() {
    return (void *) pages[last_page_index++];
}

static inline linear_address get_linear_address(uint64_t address) {
    return *((linear_address *) &address);
}

static inline void initialize_page(page_entry *entry, uint64_t address) {
    entry->address = address >> ADDRESS_BITS;
    entry->present = 1;
    entry->write_allowed = 1;
}

static inline void *get_page(page_entry *table, uint64_t entry_id) {
    page_entry *entry = &table[entry_id];
    
    if (entry->present == 1) {
        return (void *) (entry->address << ADDRESS_BITS);
    } else {
        void *new_page = allocate_page();
        initialize_page(entry, (uint64_t) new_page);
        return new_page;
    }
}

static inline void map_pt(page_entry pt[], uint64_t virtual_addr, 
        uint64_t physical_addr) 
{
    page_entry *entry = &pt[get_linear_address(virtual_addr).pt];
    initialize_page(entry, physical_addr);
}

#define CREATE_MAPPING(from_table, to_table) \
    static inline void map_ ## from_table (page_entry from_table[],     \
            uint64_t virtual_addr, uint64_t physical_addr)              \
    {                                                                   \
        void *to_table = get_page(from_table, get_linear_address(virtual_addr).from_table); \
        map_ ## to_table (to_table, virtual_addr, physical_addr);       \
    }

CREATE_MAPPING(pd, pt)
CREATE_MAPPING(pdpt, pd)
CREATE_MAPPING(pml4, pdpt)

static inline void map_memory(uint64_t virtual_addr, uint64_t physical_addr,
        uint32_t page_count)
{
    serial_print("mapping memory...\r\n");
    
    uint64_t virtual_addr_end = virtual_addr + page_count * PAGE_SIZE;
    
    uint64_t v_address = virtual_addr;
    uint64_t p_address = physical_addr;
    
    SERIAL_DUMP_HEX(virtual_addr);
    SERIAL_DUMP_HEX(physical_addr);
    SERIAL_DUMP_HEX(page_count);
    SERIAL_DUMP_HEX(virtual_addr_end);
    
    while (v_address < virtual_addr_end)
    {
        map_pml4(pml4, v_address, p_address);
        
        v_address += PAGE_SIZE;
        p_address += PAGE_SIZE;
    }
}

static const EFI_MEMORY_DESCRIPTOR *get_next_descriptor(
	const EFI_MEMORY_DESCRIPTOR *descriptor, UINT64 descriptor_size)
{
    const uint8_t *desc = ((const uint8_t *) descriptor) + descriptor_size;
    return (const EFI_MEMORY_DESCRIPTOR *) desc;
}

static inline void map_efi(EFI_MEMORY_MAP *memory_map) {
    const EFI_MEMORY_DESCRIPTOR *descriptor = memory_map->memory_map;
    const UINT64 descriptor_size = memory_map->descriptor_size;
    
    for (UINT64 i = 0; i < memory_map->memory_map_size; ++i) {
        if (descriptor->Attribute & EFI_MEMORY_RUNTIME) {
            map_memory(descriptor->PhysicalStart, descriptor->PhysicalStart,
                    descriptor->NumberOfPages);
        }
        
        descriptor = get_next_descriptor(descriptor, descriptor_size);
    }
}

static inline void map_framebuffer(framebuffer *fb) {
    void *framebuffer_base = fb->base;
    map_memory(FRAMEBUFFER_START, (uint64_t) framebuffer_base, fb->size / PAGE_SIZE + 1);
}

void enable_paging(void *omos_kernel, EFI_MEMORY_MAP *memory_map, framebuffer *fb) {
    map_memory(KERNEL_START, KERNEL_START, 256);
    serial_print("mapped kernel\r\n");
    
    map_memory(KERNEL_VIRTUAL_BASE, (uint64_t) omos_kernel, 256);
    serial_print("mapped omos kernel\r\n");
    
    map_efi(memory_map);
    serial_print("mapped efi\r\n");
    
    map_framebuffer(fb);
    serial_print("mapped framebuffer\r\n");
    
    serial_print("CR3 will point to: ");
    serial_print_hex((uint64_t) pml4);
    serial_print("\r\n");
    
    write_cr3((uint64_t) pml4);    
}
