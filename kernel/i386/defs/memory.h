/*
 * Here are some constants about standard memory mappings in IBM pc/at and 
 * compatibles.
 * In general, the first megabyte of the physical address space is left
 * untouched by the kernel. The following standard mappings apply:
 * 1. BIOS standard area goes from 0x0000 - 0x04ff (0KB-1.24KB(inclusive))
 * 2. 80286 load-all workspace (??????) goes from 0x0800-0x0866 (2KB-2.0KB)
 * 3. Free real-mode space goes from 0x0500-0xffff (1.25KB-63KB(inclusive))
 * 4. Real-mode compatibility segment goes from 0x010000-0x01ffff 
 * (64KB-127KB(inclusive))
 * 5. Free memory area goes from 0x020000-0x090000 (128KB-575KB(inclusive))
 * 5. BIOS extension area goes from 0x090000-0x09ffff (576KB-639KB(inclusive))
 * 6. Memory-mapped video devices goes from 0x0a0000-0x0bffff 
 * (640KB-767KB(inclusive))
 * 7. Some devices BIOS ROM extensions goes from 0x0c0000-0x0dffff 
 * (768KB-895KB(inclusive))
 * 8. Motherboard BIOS extensions goes from 0x0e0000-0x0fffff
 * (896KB-1023KB(inclusive))
 * 9. First 64KB of the second MB of memory is also used; goes from 
 * 0x100000-0x10ffef(1024KB-1087KB(inclusive))
 * The information was obtained from MINIX's memory.h.
 */

/*
 * The first megabyte of physical memory is usually used by the BIOS,
 * memory-mapped video devices, ROM extensions, plug and play, etc, 
 * so don't count on it for doing our mappings.
 */
#define BIOS_LIMIT 0x100000 // BIOS space ends here
#define BIOS_IVT_START 0x0000 // start of interrupt vector table formed by BIOS
#define BIOS_IVT_END 0x03ff // end of interrupt vector table formed by BIOS
#define BIOS_DATA_START 0x0400 // start of BIOS data area
#define BIOS_DATA_END 0x04ff // end of BIOS data area

/*
 * The BIOS area goes from physical address 0 to 640KB.
 * This area is divided in two spaces:
 * The first area is the first 20KB, and is absolutely dedicated to the BIOS.
 * The second area starts at 0x0500 through 0x09ffff, that is, byte 20KB to
 * 640KB. From this space, only the top 1KB, that is, from 0x090000 through
 * 0x09ffff, and is used by the BIOS to store additional data.
 */
#define BASE_MEM_START 0x000000
#define BASE_MEM_FREE_SPACE_START 0x020000
#define BASE_MEM_FREE_SPACE_1_START 0x0500
#define BASE_MEM_FREE_SPACE_1_END 0xffff
#define BASE_MEM_FREE_SPACE_2_START 0x020000
#define BASE_MEM_FREE_SPACE_2_END 0x08ffff
#define BASE_MEM_UPPER_START 0x090000
#define BASE_MEM_UPPER_END 0x09ffff
#define BASE_MEM_END 0x09ffff

/*
 * The following space is reserved for memory-mapped video devices.
 */
#define VIDEO_VRAM_START 0xa0000 // start of video ram area
#define VIDEO_VRAM_END 0xbffff // start of video ram area
#define VIDEO_GRAPHICS_RAM_START 0xa0000
#define VIDEO_GRAPHICS_RAM_END 0xaffff
#define VIDEO_CHAR_MONO_START 0xb0000
#define VIDEO_CHAR_MONO_END 0xb7fff
#define VIDEO_CHAR_COLOR_START 0xb8000
#define VIDEO_CHAR_COLOR_END 0xbffff

/*
 * The following space is reserved for some memory-mapped ROM extensions.
 */
#define ROM_EXT_START 0xc0000
#define ROM_VIDEO_EXT_START 0xc0000
#define ROM_VIDEO_EXT_END 0xc7fff
#define ROM_IDE_EXT_START 0xc8000
#define ROM_IDE_EXT_END 0xcbfff
#define ROM_EXT_END 0xd0000

/*
 * The following space is reserved for some memory-mapped motherboard 
 * ROM extensions.
 */
#define MB_ROM_EXT_START 0xe0000
#define MB_PNP_ROM_VIDEO_EXT_START 0xe0000
#define MB_PNP_ROM_VIDEO_EXT_END 0xeffff
#define MB_SYS_ROM_VIDEO_EXT_START 0xf0000
#define MB_SYS_ROM_VIDEO_EXT_END 0xfffff
#define MB_ROM_EXT_END 0xfffff
