#define NULL 0
#define KERN_ENT 0
#define MEM_ENT 0
#define PROC_ENT 1
#define FS_ENT 2
#define IO_ENT 3
#define OTHER 4
#define HEX 1
#define DEC 2

/* define messages' flags */
#define RESPONSE 0x01
#define REQUEST 0x02
#define ACK 0x04
#define PROC_POOL 0x08
#define FS_POOL 0x10
#define IO_POOL 0x20
#define USR_POOL 0x40
#define PAGS4K 0x80

/* define messages' commands (mutually exclusive) */
#define NONE 0
#define ALLOC 1
#define NEW_ENTRY 2
#define CPUID 3
#define IOCTL 80

typedef int word;
typedef char byte;
typedef unsigned char ubyte_t;
typedef unsigned int uint_t;
typedef unsigned short ushort_t;
typedef unsigned int uaddr_t;
typedef unsigned int usec_t;
typedef unsigned int usize_t;
byte buffer[512];
byte *ptr_buf;
/*
 * This structure defines an data structure passed to the kernel
 */
struct set_args {
	char *kern_path;
	char *proc_serv_path;
	char *vm_serv_path;
	char *fs_serv_path;
	char *dev_serv_path;
	byte debug;
	ubyte_t num_servs;
};

struct msg_fmt {
	word cmd;
	word flags;
	word number;
	uaddr_t *ptr_smwh; // pointer somewhere
};
/*
 * Udp-like protocol for communication between processes.
 */
struct com_fmt {
	ushort_t unit_size;
	ushort_t src_port;
	ushort_t dst_port;
	ushort_t dgr_len;
	ushort_t chksum;
	ushort_t ent;
	struct msg_fmt payld;
};
#define ENT_OFF 0x80
#define MEM_IN 0
#define MEM_OUT 1
#define PROC_IN 2
#define PROC_OUT 3
#define FS_IN 4
#define FS_OUT 5
#define IO_IN 6
#define IO_OUT 7
