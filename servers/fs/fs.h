#ifndef COMMON
#define COMMON
#include "../../libcommon/com.h"
#endif

struct com_fmt *pool_in_out[2];

/*
 * Super block for an FFS file system in memory.
 */
struct fs {
	uint_t	 fs_firstfield;		/* historic file system linked list, */
	uint_t	 fs_unused_1;		/*     used for incore super blocks */
	uint_t  fs_sblkno;		/* addr of super-block in filesys */
	uint_t  fs_cblkno;		/* offset of cyl-block in filesys */
	uint_t  fs_iblkno;		/* offset of inode-blocks in filesys */
	uint_t  fs_dblkno;		/* offset of first data after cg */
	uint_t	 fs_old_cgoffset;	/* cylinder group offset in cylinder */
	uint_t	 fs_old_cgmask;		/* used to calc mod fs_ntrak */
	uint_t	 fs_old_time;		/* last time written */
	uint_t	 fs_old_size;		/* number of blocks in fs */
	uint_t	 fs_old_dsize;		/* number of data blocks in fs */
	uint_t	 fs_ncg;		/* number of cylinder groups */
	uint_t	 fs_bsize;		/* size of basic blocks in fs */
	uint_t	 fs_fsize;		/* size of frag blocks in fs */
	uint_t	 fs_frag;		/* number of frags in a block in fs */
/* these are configuration parameters */
	uint_t	 fs_minfree;		/* minimum percentage of free blocks */
	uint_t	 fs_old_rotdelay;	/* num of ms for optimal next block */
	uint_t	 fs_old_rps;		/* disk revolutions per second */
/* these fields can be computed from the others */
	uint_t	 fs_bmask;		/* ``blkoff'' calc of blk offsets */
	uint_t	 fs_fmask;		/* ``fragoff'' calc of frag offsets */
	uint_t	 fs_bshift;		/* ``lblkno'' calc of logical blkno */
	uint_t	 fs_fshift;		/* ``numfrags'' calc number of frags */
/* these are configuration parameters */
	uint_t	 fs_maxcontig;		/* max number of contiguous blks */
	uint_t	 fs_maxbpg;		/* max number of blks per cyl group */
/* these fields can be computed from the others */
	uint_t	 fs_fragshift;		/* block to frag shift */
	uint_t	 fs_fsbtodb;		/* fsbtodb and dbtofsb shift constant */
	uint_t	 fs_sbsize;		/* actual size of super block */
	uint_t	 fs_spare1[2];		/* old fs_csmask */
					/* old fs_csshift */
	uint_t	 fs_nindir;		/* value of NINDIR */
	uint_t	 fs_inopb;		/* value of INOPB */
	uint_t	 fs_old_nspf;		/* value of NSPF */
/* yet another configuration parameter */
	uint_t	 fs_optim;		/* optimization preference, see below */
/* these fields are derived from the hardware */
	uint_t	 fs_old_npsect;		/* # sectors/track including spares */
	uint_t	 fs_old_interleave;	/* hardware sector interleave */
	uint_t	 fs_old_trackskew;	/* sector 0 skew, per track */
/* fs_id takes the space of the unused fs_headswitch and fs_trkseek fields */
	uint_t	 fs_id[2];		/* unique file system id */
/* sizes determined by number of cylinder groups and their sizes */
	uint_t  fs_old_csaddr;		/* blk addr of cyl grp summary area */
	uint_t	 fs_cssize;		/* size of cyl grp summary area */
	uint_t	 fs_cgsize;		/* cylinder group size */
/* these fields are derived from the hardware */
	uint_t	 fs_spare2;		/* old fs_ntrak */
	uint_t	 fs_old_nsect;		/* sectors per track */
	uint_t	 fs_old_spc;		/* sectors per cylinder */
	uint_t	 fs_old_ncyl;		/* cylinders in file system */
	uint_t	 fs_old_cpg;		/* cylinders per group */
	uint_t	 fs_ipg;		/* inodes per group */
	uint_t	 fs_fpg;		/* blocks per group * fs_frag */
};
struct fs_mem {
	struct pool_mgmt *ptr_pool_fs;
	uint_t granularities[1];
} fs_mem;
