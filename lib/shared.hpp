#pragma once

#include <iostream>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

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
#define INODES 10
#define BLOCKS 20
#define MAX_DIR 16

#define DEBUG_MODE true
#define LOG(x)      \
	if (DEBUG_MODE) \
	std::cout << "[ INFO ] " << x << std::endl

/*
A file system consists of a sequence of logical blocks, each containing 512, 1024,
2048, or any convenient multiple of 512 bytes, depending on the system
implementation.



*/