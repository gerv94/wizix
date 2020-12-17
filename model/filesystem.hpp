#include "inode.hpp"
#include "superblock.hpp"
#include "bootblock.hpp"
#include "directoryitem.hpp"

class filesystem
{
private:
	/* data */
	FILE *fw;				//Virtual disk file write file pointer
	FILE *fr;				//Virtual disk file read file pointer

	int Superblock_StartAddr;  //Super block Offset address, occupying a disk block
	int InodeBitmap_StartAddr; //Inode bitmap Offset address, occupies two disk blocks, monitors the status of up to 1024 inodes
	int BlockBitmap_StartAddr; //block bitmap Offset address, occupies 20 disk blocks, monitors the status of 10240 disk blocks (5120KB) at most
	int Inode_StartAddr;	   //The offset address of the inode node area, occupying INODE_NUM/(BLOCK_SIZE/INODE_SIZE) disk blocks
	int Block_StartAddr;	   //Offset address of block data area, occupying INODE_NUM disk blocks
	int Sum_Size;			   //Virtual disk file size
	int File_Max_Size;		   //The maximum size of a single file, 10 direct blocks, first-level indirect blocks, and second-level indirect blocks

	bool inode_bitmap[INODE_NUM]; //inode bitmap
	bool block_bitmap[BLOCK_NUM]; //Disk block bitmap

	char Cur_User_Name[110];  //Currently logged in user name
	char Cur_Group_Name[110]; //Current login user group name

	//:::::::::::::::::::::::::::::::::::::::::::::::::::::

	BootBlock bootblock; // BootBlock structure just empty
	SuperBlock superblock; // SuperBlock structure

	FILE *diskptr;


public:
	filesystem();
	~filesystem();

	void init();
	void mountDisk();
	void createDisk(const char *);
	int getInodeListBlockCount();
	void createRootDir();

	bool format(const char *); //Formats the virtual disk file

	int InodeAlloc();
	int BlockAlloc();
};


/*

::::::: Documentation :::::::

A file system has the following structure:

BootBlock	SuperBlock	iNodeList				DataBlocks
-------------------------------------------------------------------------
|			|			|			...			|			...			|
|			|			|			...			|			...			|
|			|			|			...			|			...			|
|			|			|			...			|			...			|
|			|			|			...			|			...			|
-------------------------------------------------------------------------

* The boot block occupies the beginning of a file system, typically the first sector,
and may contain the bootstrap code that is read into the machine to boot, or
initialize, the operating system. Although only one boot block is needed to boot
the system, every file system has a (possibly empty) boot block. 

* The super block describes the state of a file system -- how large it is, how
many files it can store, where to find free space on the file system, and other
information. 

* The inode list is a list of inodes that follows the super block in the file system.
Administrators specify the size of the inode list when configuring a file system.
The kernel references inodes by index into the inode list. One inode is the root
inode of the file system: it is the inode by which the directory structure of the
file system is accessible after execution of the mount system call

* The data blocks start at the end of the inode list and contain file data and
administrative data. An allocated data block can belong to one and only one
file in the file system.
*/