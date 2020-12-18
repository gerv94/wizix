#include "inode.hpp"
#include "superblock.hpp"
#include "bootblock.hpp"
#include "directoryitem.hpp"

class filesystem
{
private:

	BootBlock bootblock;   // BootBlock structure just empty
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
	Inode iget(unsigned short);
	unsigned short namei(Inode, const char *); /* convert path name to inode */

	void startConsole();
	void touch(Inode, const char *); // creates a file
	void mkdir(unsigned short, const char *); // creates a directory
	void ls(Inode);					 // lists current inode directory items
	void cd(Inode);					 // change directory
	void rm(Inode, const char *);	 // removes a file inode
	void rmdir(Inode, const char *); // removes an empty directory inode
	void vi(Inode, const char *); // removes an empty directory inode
	void cat(Inode, const char *); // removes an empty directory inode

	void updateSuperBlock();

	unsigned short getNextFreeInode();
	unsigned short getNextFreeBlock();
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