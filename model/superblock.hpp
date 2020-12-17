#include "../lib/shared.hpp"

#define INODES 10
#define BLOCKS 12

struct SuperBlock
{
	// ::::::::::::::::::::::::::::::::::::::::::::::::

	int filesystem_size; // BootBlock + SuperBlock + InodeListBlock + DataBlocks
	int free_blocks_num;
	int next_free_block;

	int inodelist_size;
	int free_inodes_num;
	int next_free_inode;

	bool modified;

	int free_blocks[INODES] = {0};
	int free_inodes[BLOCKS] = {0};
};

/*

::::::: Documentation :::::::


The super block consists of the following fields:

* the size of the file system,
* the number of free blocks in the file system,
* a list of free blocks available on the file system,
* the index of the next free block in the free block list,

* the size of the inode list,
* the number of free inodes in the file system,
* a list of free inodes in the file system,
* the index of the next free inode in the free inode list,

* lock fields for the free block and free inode lists,
* a flag indicating that the super block has been modified. 

*/