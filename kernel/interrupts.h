#ifndef INTERRUPTS_H
#define INTERRUPTS_H

#include <stddef.h>
#include <stdint.h>

#define PIC1		0x20		/* IO base address for master PIC */
#define PIC2		0xA0		/* IO base address for slave PIC */
#define PIC1_COMMAND	PIC1
#define PIC1_DATA	(PIC1+1)
#define PIC2_COMMAND	PIC2
#define PIC2_DATA	(PIC2+1)
#define PIC_EOI		0x20		/* End-of-interrupt command code */

void enable_interrupts();

uint8_t read_port(uint16_t port);
void write_port(uint16_t port, uint8_t value);

#endif /* INTERRUPTS_H */

