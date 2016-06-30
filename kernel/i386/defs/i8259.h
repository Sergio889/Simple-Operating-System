/*
 * Definitions for some constants for programming the 8259 IPC.
 */
#define INT_MST 0x20 /* IO port for master 8259 */
#define INT_SLV 0xA0 /* IO port for slave 8259 */

#define ICW1_LEVEL 0x19 /* level-triggered, cascade, need ICW4 */
#define ICW1_EDGE 0x11 /* edge-trigerred, cascade, need ICW4 */
#define ICW2_MST 0x50 /* relocate because of BIOS locations */
#define ICW2_SLV 0x70 /* must not relocate */
#define ICW4_MST, 0x0D /* ~FNM, buffered, master, 8086 */
#define ICW4_SLV, 0x09 /* ~FNM, buffered, slave,  8086 */
