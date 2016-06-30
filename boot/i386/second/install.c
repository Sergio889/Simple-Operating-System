/*
 * Writes the second program to disk plus a magic sector.
 */
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>

#define SEC_SIZE 512

static int part_fd;
static int copy_fd;
static char block[513];
static unsigned int sectors_written;
static void instructions();

int
main(int argc, char *argv[])
{
	int i, result;
	char empty_str[512];

	printf("\nInstalling boot program...");
	printf("\n==========================================================\n");
	for (i=0; i<=511; i++)
		empty_str[i] = 'a';
	empty_str[512] = '\0';
	
	if (argc != 3) {
		instructions();
		return 0;
	}
	
	if ( (part_fd=open(argv[1], O_RDWR | O_SYNC)) == -1) {
		perror("open()");
		return 0;
	}
	printf("Opened partition special-file '%s'\n", argv[1]);
	if ( (copy_fd=open(argv[2], O_RDWR | O_SYNC)) == -1) {
		perror("open()");
		return 0;
	}
	printf("Opened binary file '%s'\n", argv[2]);

	for (i=0; i<200; i++) {
		if ( (result=read(copy_fd, block, SEC_SIZE)) == -1) {
			perror("read()");
			return 0;
		}
		if (result) {
			write(part_fd, block, SEC_SIZE);
			sectors_written++;
		} else {
			write(part_fd, empty_str, SEC_SIZE);
			printf("Written magic block to sector %d of '%s'\n", sectors_written, argv[1]);
			break;
		}
	}
	printf("%d sectors written from '%s' to '%s'\n", sectors_written-1, argv[2], argv[1]);
}

void
instructions()
{
	printf("Partition+binary\n");
}
