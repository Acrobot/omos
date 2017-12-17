#include "interrupts.h"
#include "../serial.h"

#define PACKED __attribute__((packed))

typedef struct {
	uint16_t limit;
	uint64_t offset;
} PACKED idtr;

typedef struct {
	uint16_t offset_lowerbits : 16;
	uint16_t selector : 16;
        uint8_t ist : 8;
        uint8_t gate_type : 4;
        uint8_t z : 1;
        uint8_t dpl : 2;
        uint8_t present : 1;
	uint64_t offset_higherbits : 48;
	uint32_t zero : 32;
} PACKED idt_entry;

_Static_assert(sizeof(idtr) == 10, "IDTR register has to be 40 bits long");

uint8_t read_port(uint16_t port) {
    uint8_t data;
    __asm__ volatile("inb %w1,%b0" : "=a" (data) : "d"(port));
    return data;
}

void write_port(uint16_t port, uint8_t value) {
    __asm__ volatile("outb %b0,%w1" : : "a" (value), "d"(port));
}

static inline void load_idt(idtr *idtr) {
    __asm__ volatile("lidtq %0" : : "m" (*idtr));
    __asm__ volatile("sti");
}

/* http://wiki.osdev.org/Inline_Assembly/Examples#I.2FO_access */
static inline void io_wait(void) {
    /* TODO: This is probably fragile. */
    __asm__ volatile ( "jmp 1f\n\t"
                     "1:jmp 2f\n\t"
                     "2:" );
}

/* Handling IDT */
#define IDT_SIZE 256
#define SYS_CODE64_SEL 0x38 // https://github.com/tianocore/edk/blob/master/Sample/Universal/DxeIpl/Pei/x64/LongMode.asm#L281
idt_entry IDT[IDT_SIZE];

/* Handling keyboard */
extern void keyboard_handler();
#define IRQ1 0x21

void initialize_keyboard(idt_entry *entry) {
    uint64_t keyboard_address = ((uint64_t) keyboard_handler);
    entry->offset_lowerbits = keyboard_address & 0xffff;
    entry->offset_higherbits = (keyboard_address & 0xffffffffffff0000) >> 16;
    entry->selector = SYS_CODE64_SEL;
    entry->zero = 0;
    entry->ist = 0;
    entry->z = 0;
    entry->dpl = 0;
    entry->present = 1;
    entry->gate_type = 0xe; // Interrupt gate
}

 
#define ICW1_ICW4	0x01		/* ICW4 (not) needed */
#define ICW1_SINGLE	0x02		/* Single (cascade) mode */
#define ICW1_INTERVAL4	0x04		/* Call address interval 4 (8) */
#define ICW1_LEVEL	0x08		/* Level triggered (edge) mode */
#define ICW1_INIT	0x10		/* Initialization - required! */
 
#define ICW4_8086	0x01		/* 8086/88 (MCS-80/85) mode */
#define ICW4_AUTO	0x02		/* Auto (normal) EOI */
#define ICW4_BUF_SLAVE	0x08		/* Buffered mode/slave */
#define ICW4_BUF_MASTER	0x0C		/* Buffered mode/master */
#define ICW4_SFNM	0x10		/* Special fully nested (not) */
 
/*
arguments:
    offset1 - vector offset for master PIC
            vectors on the master become offset1..offset1+7
    offset2 - same for slave PIC: offset2..offset2+7
 * 
 * Taken from http://wiki.osdev.org/8259_PIC
*/
void remap_pic(int offset1, int offset2) {
    unsigned char a1, a2;

    a1 = read_port(PIC1_DATA); // save masks
    a2 = read_port(PIC2_DATA);

    write_port(PIC1_COMMAND, ICW1_INIT+ICW1_ICW4);  // starts the initialization sequence (in cascade mode)
    io_wait();
    write_port(PIC2_COMMAND, ICW1_INIT+ICW1_ICW4);
    io_wait();
    write_port(PIC1_DATA, offset1);     // ICW2: Master PIC vector offset
    io_wait();
    write_port(PIC2_DATA, offset2);     // ICW2: Slave PIC vector offset
    io_wait();
    write_port(PIC1_DATA, 4);           // ICW3: tell Master PIC that there is a slave PIC at IRQ2 (0000 0100)
    io_wait();
    write_port(PIC2_DATA, 2);           // ICW3: tell Slave PIC its cascade identity (0000 0010)
    io_wait();

    write_port(PIC1_DATA, ICW4_8086);
    io_wait();
    write_port(PIC2_DATA, ICW4_8086);
    io_wait();

    write_port(PIC1_DATA, a1);   // restore saved masks.
    write_port(PIC2_DATA, a2);
}

void enable_interrupts() {
    serial_print("Initializing keyboard...\r\n");
    initialize_keyboard(&IDT[IRQ1]);
    
    idtr idtr;
    idtr.limit = (sizeof(idt_entry) * IDT_SIZE) - 1;
    idtr.offset = (uint64_t) IDT;

    serial_print("Loading IDTR...\r\n");
    load_idt(&idtr);
    
    serial_print("Reprogramming PIC...\r\n");
    remap_pic(0x20, 0x28);
    
    /* Masking IRQ to only support IRQ1 (keyboard) */
    serial_print("Masking IRQ...\r\n");
    write_port(IRQ1, 0xFD);
}
