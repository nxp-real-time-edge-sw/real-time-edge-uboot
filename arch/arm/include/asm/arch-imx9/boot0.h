/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2026 NXP
 */

	b	1f
#if defined(CONFIG_EMBEDDED_FASTBOOT_BLOB)
	/* Align blob at offset of 16 bytes */
	.align	4
	/* Reserve CONFIG_EMBEDDED_FASTBOOT_BLOB_SIZE bytes and fill zero */
	.long	0x54425346	/* Magic number: "FSBT" in ASCII (little-endian) */
	.long	CONFIG_EMBEDDED_FASTBOOT_BLOB_SIZE	/* total size of reserved blob space */
	.zero	(CONFIG_EMBEDDED_FASTBOOT_BLOB_SIZE - 8)
	.align	3
#endif
1:
	/*
	 * Add board specific code at here to avoid to change
	 * offset of fastboot blob
	 */
	b	reset

