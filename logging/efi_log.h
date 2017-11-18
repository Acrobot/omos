#ifndef EFI_LOG_H
#define EFI_LOG_H

#include <wchar.h>

#define EFI_CHECK(expr) \
do { \
	EFI_STATUS __status = (expr); \
	Print(L ## #expr L"\r\n"); \
	PrintStatus(__status); \
} while(0)

void PrintStatus(EFI_STATUS status);

#endif /* EFI_LOG_H */

