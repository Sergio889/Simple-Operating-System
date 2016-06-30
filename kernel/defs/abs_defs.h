/*
 * The following are definitions of static data regarding requests and
 * services. Adding a request and/or services should be simple.
 */
#define	NPROCIOMST	/* # of requests from PROC to IO */
#define	NPROCMEMMST	/* # of requests from PROC to MEM */
#define	NPROCINTOPS	/* # of internal PROC operations */
#define	NIOPROCSLV	/* # of services from IO to PROC */
#define	NIODEVMST	/* # of requests from IO to DEV */
#define	NDEVIOSLV	/* # of services from DEV to IO */
#define	NDEVBLKMST	/* # of requests from DEV to BLK */
#define	NDEVTTYMST	/* # of requests from DEV to TTY */
#define	NIOFSMST	/* # of requests from IO to FS */
#define	NMEMPROCSLV	/* # of services from MEM to PROC */
#define	NMEMVMMST	/* # of requests from MEM to memory subsystem */
