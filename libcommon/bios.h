/*
 * Common structures and functions related to BIOS.
 */
/*
 * Disk Address Packet for extended r/w.
 */
struct ext_rw_dap {
	byte dap_size; // size of packet
	byte res_1; // reserved; must be 0
	byte dap_blknum; // number of blocks to transfer
	byte res_2; // reserved; must be 0
	uint_t dap_trans_buf; // seg:off
	uint_t dap_abs_lba[2]; // 64-bits for LBA
};
