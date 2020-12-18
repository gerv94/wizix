#include "../lib/shared.hpp"

struct SuperBlock
{
	// ::::::::::::::::::::::::::::::::::::::::::::::::

	int filesystem_size; // 4 bytes BootBlock + SuperBlock + InodeListBlock + DataBlocks
	int free_blocks_num; 
	int next_free_block;
	int inodelist_size;
	int free_inodes_num;
	int next_free_inode;

	bool modified;

	int free_blocks[BLOCKS] = {0}; // 0 means free, 1 occupied
	int free_inodes[INODES] = {0}; // 0 means free, 1 occupied
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