#pragma once

#include <iostream>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define MFS_BUFFER 100000 // 100K, virtual disk buffer
#define FILE_BUFFER 10000 // 10K, file buffer
#define INODE_SIZE 128	  // The size of the inode node is 128B. Note: sizeof(Inode) cannot exceed this value
#define MAX_NAME_SIZE 28  // Maximum name length, the length should be less than this size

#define INODE_NUM 640		//Number of inode nodes, up to 64 files
#define BLOCK_NUM 10240		//Block number, 10240 * 512B = 5120KB
#define BLOCKS_PER_GROUP 64 //Free block stack size, how many disk block addresses can be stored in a free stack

#define MODE_DIR 01000	//Directory identification
#define MODE_FILE 00000 //File identification

#define FILE_DEF_PERMISSION 0664 //File default permissions
#define DIR_DEF_PERMISSION 0755	 //Directory default permissions

// :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

#define EXT4_BAD_INO             1      /* Bad blocks inode */
#define EXT4_ROOT_INO            2      /* Root inode */

#define EXT4_USR_QUOTA_INO       3      /* User quota inode */
#define EXT4_GRP_QUOTA_INO       4      /* Group quota inode */
#define EXT4_BOOT_LOADER_INO     5      /* Boot loader inode */
#define EXT4_UNDEL_DIR_INO       6      /* Undelete directory inode */
#define EXT4_RESIZE_INO          7      /* Reserved group descriptors inode */
#define EXT4_JOURNAL_INO         8      /* Journal inode */


#define DISK_FILE "filesystem.wz"
#define BLOCK_SIZE 512 // The block number size is 512B

#define DEBUG_MODE true
#define LOG(x)      \
	if (DEBUG_MODE) \
	std::cout << "[ INFO ] " << x << std::endl

/*
A file system consists of a sequence of logical blocks, each containing 512, 1024,
2048, or any convenient multiple of 512 bytes, depending on the system
implementation.



*/