#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>


#include "model/inode.hpp"
/*
Using disk blocks of 1024 bytes, 1024 chars

Assume that a logical block on the file system holds lK bytes
and that a block number is addressable by a 32 bit (4 bytes) integer.

*/

u_char boot_block[1024];
u_char super_block[1024];
inode inode_list[10];
u_char data_blocks[1024];

int main(int argc, char const *argv[])
{
	system("touch ./test.disk");

	int fd = open("./test.disk", O_WRONLY);
	if (fd == -1)
	{
		printf("Failed to open file");
		return 0;
	}

	write(fd, &boot_block, sizeof(boot_block));
	close(fd);

	printf("Test run succesfully\n");

	return 0;
}
