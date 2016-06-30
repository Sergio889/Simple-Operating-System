/*
 * These are some absolute physical addresses within the BIOS that have
 * important information about the system.
 * The information was obtained from MINIX's bios.h.
 */

/* 
 * The base IO address for the 6845 controller is located at 0x463 and
 * is a half word.
 */
#define VDU_CRT_BASE_ADDR       0x463
#define VDU_CRT_BASE_SIZE       2L
/*
 * The following addresses are part of the real-mode Interrupt Vector Table
 * created by the BIOS, but are not pointers to interrupt handlers, but
 * data structures.
 */
#define BIOS_EQUIP_CHECK_ADDR      0x0044 
#define BIOS_EQUIP_CHECK_SIZE      4L

#define BIOS_VIDEO_PARAMS_ADDR     0x0074        
#define BIOS_VIDEO_PARAMS_SIZE     4L

#define BIOS_FLOP_PARAMS_ADDR      0x0078     
#define BIOS_FLOP_PARAMS_SIZE      4L
 
#define BIOS_HD0_PARAMS_ADDR       0x0104 /* disk 0 parameters */
#define BIOS_HD0_PARAMS_SIZE       4L

#define BIOS_HD1_PARAMS_ADDR       0x0118 /* disk 1 parameters */
#define BIOS_HD1_PARAMS_SIZE       4L

/* Soft reset flags to control shutdown. */
#define SOFT_RESET_FLAG_ADDR    0x472   /* soft reset flag on Ctl-Alt-Del */
#define SOFT_RESET_FLAG_SIZE    2L  
#define   STOP_MEM_CHECK        0x1234  /* bypass memory tests & CRT init */
#define   PRESERVE_MEMORY       0x4321  /* preserve memory */
#define   SYSTEM_SUSPEND        0x5678  /* system suspend */
#define   MANUFACTURER_TEST     0x9ABC  /* manufacturer test */
#define   CONVERTIBLE_POST      0xABCD  /* convertible POST loop */
                            /* ... many other values are used during POST */

/* Hard disk parameters. (Also see BIOS interrupt vector table above.) */
#define NR_HD_DRIVES_ADDR       0x475  /* number of hard disk drives */ 
#define NR_HD_DRIVES_SIZE       1L

/* Parallel ports (LPT1-LPT4) timeout values. */
#define LPT1_TIMEOUT_ADDR       0x478   /* time-out value for LPT1 */
#define LPT1_TIMEOUT_SIZE       1L  
#define LPT2_TIMEOUT_ADDR       0x479   /* time-out value for LPT2 */
#define LPT2_TIMEOUT_SIZE       1L  
#define LPT3_TIMEOUT_ADDR       0x47A   /* time-out value for LPT3 */
#define LPT3_TIMEOUT_SIZE       1L  
#define LPT4_TIMEOUT_ADDR       0x47B   /* time-out for LPT4 (except PS/2) */
#define LPT4_TIMEOUT_SIZE       1L  

/* Serial ports (COM1-COM4) timeout values. */
#define COM1_TIMEOUT_ADDR       0x47C   /* time-out value for COM1 */
#define COM1_TIMEOUT_SIZE       1L  
#define COM2_TIMEOUT_ADDR       0x47D   /* time-out value for COM2 */
#define COM2_TIMEOUT_SIZE       1L  
#define COM3_TIMEOUT_ADDR       0x47E   /* time-out value for COM3 */
#define COM3_TIMEOUT_SIZE       1L  
#define COM4_TIMEOUT_ADDR       0x47F   /* time-out value for COM4 */
#define COM4_TIMEOUT_SIZE       1L  

/* Video controller (VDU). */
#define VDU_SCREEN_ROWS_ADDR    0x484   /* screen rows (less 1, EGA+)*/
#define VDU_SCREEN_ROWS_SIZE    1L  
#define VDU_FONTLINES_ADDR      0x485   /* point height of char matrix */
#define VDU_FONTLINES_SIZE      2L 

/* Video controller (VDU). */
#define VDU_VIDEO_MODE_ADDR     0x49A   /* current video mode */
#define VDU_VIDEO_MODE_SIZE     1L  

/* PART III --
 * The motherboard BIOS memory contains some known values that are currently 
 * in use. Other sections in the upper memory area (UMA) addresses vary in 
 * size and locus and are not further defined here. A rough map is given in 
 * <ibm/memory.h>. 
 */

/* Machine ID (we're interested in PS/2 and AT models). */
#define MACHINE_ID_ADDR         0xFFFFE /* BIOS machine ID byte */
#define MACHINE_ID_SIZE         1L
#define   PS_386_MACHINE        0xF8    /* ID byte for PS/2 modela 70/80 */
#define   PC_AT_MACHINE         0xFC    /* PC/AT, PC/XT286, PS/2 models 50/60 */
