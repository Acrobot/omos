SECTIONS
{
	/* Start putting sections at 1MiB, conventional place for bootloaders */
	. = 1M;

	.text BLOCK(4K) : ALIGN(4K)
	{
		*(.head)
		*(.text)
	}

	/* Read-only data. */
	.rodata BLOCK(4K) : ALIGN(4K)
	{
		*(.rodata)
	}

	/* Read-write data (initialized) */
	.data BLOCK(4K) : ALIGN(4K)
	{
		*(.data)
	}

	.bss :
	{
		*(.stack)
		*(.bss)
		_kernel_end = .;
	}
}
