/*
 * This template should support general aspects relative to file systems.
 * It only deals with the mechanisms to communicate with other templates
 * and the rest of the kernel. File system designs are done exclusively
 * by the user server.
 */

/*
 * One instance per file system.
 */
struct fs_abs {
	int		fs_flags;
	char	fs_type; /* type of user level server in use */

};
