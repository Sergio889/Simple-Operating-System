/*
 * One instance per device.
 */
struct dev_abs {
	char	dev_type;	/* block, terminal... */
	int		dev_flags;	/* memory-mapped... */

	void	*dev_obj;

};
