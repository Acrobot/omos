#ifndef CONSOLE_H
#define CONSOLE_H

#include "../kernel/memory_map.h"

void console_init(framebuffer *framebuffer);
void console_print(const char *message);

#endif /* CONSOLE_H */

