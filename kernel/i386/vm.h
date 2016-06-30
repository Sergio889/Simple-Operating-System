/*
 * Initial mappings control structure.
 */
struct init_vm_ctrl {
	unsigned int pag_dir_cur;
	unsigned int pag_tabl_cur;
	short pg_dir_ents;
	short pg_tabl_ents;
} vm_ctrl_init;
