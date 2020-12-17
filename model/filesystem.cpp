#include "filesystem.hpp"
#include "../lib/shared.hpp"

#include <math.h>

filesystem::filesystem(/* args */)
{
}

filesystem::~filesystem()
{
}

void filesystem::init()
{
	// Initialize filesystem
	// Check if file disk exists

	LOG("Checking if file system disk exists...");
	LOG("Size of boot-block: " << sizeof(BootBlock) << " Bytes");
	LOG("Size of super-block: " << sizeof(SuperBlock) << " Bytes");
	LOG("Size of inode: " << sizeof(Inode) << " Bytes");

	if (access(DISK_FILE, F_OK) != -1)
	{
		LOG("Disk file exists");
		mountDisk();
	}
	else
	{
		LOG("Disk file does not exists");
		createDisk(DISK_FILE); // TODO: allow user to input file name
	}
}
void filesystem::mountDisk()
{
	LOG("Mounting disk");

	FILE *fr = fopen(DISK_FILE, "rb");

	fread(bootblock.data, BLOCK_SIZE, 1, fr);

	for (int i = 0; i < BLOCK_SIZE; i++)
	{
		printf("%c", bootblock.data[i]);
	}

	printf("\n");

	fclose(fr);
}
void filesystem::createDisk(const char *disk_file)
{
	LOG("Creating disk");

	char buffer[BLOCK_SIZE] = {0};
	diskptr = fopen(disk_file, "wb");
	memset(buffer, 0, BLOCK_SIZE); // intialize null buffer

	// Using buffer to initialize whole disk as NULL
	/* Using buffer to initialize whole disk as NULL  */
    for (int i = 0; i < BLOCKS; ++i)
        fwrite(buffer, 1, BLOCK_SIZE, diskptr);

	// Initializing SuperBlock

	int inodeListBlocks = getInodeListBlockCount();
	int datablocks = BLOCKS - 2 - inodeListBlocks; // 2 because of Boot and Super Blocks
	int nextFirstblock = 2 + inodeListBlocks + 1;  // 2 because of Boot and Super Blocks

	superblock.filesystem_size = BLOCKS * BLOCK_SIZE;

	superblock.free_blocks_num = datablocks;
	superblock.next_free_block = nextFirstblock; // Offset 0 from DataBlock

	superblock.inodelist_size = INODES;
	superblock.free_inodes_num = INODES;
	superblock.next_free_inode = EXT4_ROOT_INO; // Offset 2 from inodelist

	superblock.modified = false;

	LOG("File System size: " << superblock.filesystem_size << " bytes");
	LOG("Free Blocks: " << superblock.free_blocks_num);
	LOG("Next first block: " << nextFirstblock);

	LOG("Inode List size: " << superblock.inodelist_size);
	LOG("Free Inodes: " << superblock.free_inodes_num);
	LOG("Next first inode: " << superblock.next_free_inode);

	LOG("Inode List Blocks: " << inodeListBlocks << " blocks");

	for (int i = 0; i < BLOCKS; i++)
	{
		superblock.free_blocks[i] = 0;
	}

	for (int i = 0; i < INODES; i++)
	{
		superblock.free_inodes[i] = 0;
	}

	// Storing BootBlock
	for (int i = 0; i < BLOCK_SIZE; i++){
		bootblock.data[i] = '*';
	}
	fseek(diskptr, 0, SEEK_SET);
	fwrite(&bootblock, sizeof(BootBlock), 1, diskptr);

	// Storing SuperBlock after BootBlock
	fseek(diskptr, BLOCK_SIZE, SEEK_SET);
	fwrite(&superblock, sizeof(SuperBlock), 1, diskptr);

	// Storing Inode list after SuperBlock
	//createRootDir();

	fclose(diskptr);

	LOG("File system disk created");
}

void filesystem::createRootDir()
{
	// Storing root Inode
	Inode root_inode;
	time_t now = time(0);
	root_inode.own_usr = 0; // meaning root
	root_inode.own_grp = 0; // meaning root
	root_inode.type = DIRECTORY;
	root_inode.permissions = 755; // d rwx r-x r-x
	root_inode.modify_time = now;
	root_inode.access_time = now;
	root_inode.inode_time = now;
	root_inode.links = 0;
	root_inode.content[0] = superblock.next_free_block;
	root_inode.size = BLOCK_SIZE;

	// store dir content
	DirectoryItem directories[2];
	directories[0].inode = EXT4_ROOT_INO;
	sprintf(directories[0].name, ".");

	directories[1].inode = EXT4_ROOT_INO;
	sprintf(directories[1].name, "..");

	//Store content on next free_block
	fseek(diskptr, root_inode.content[0] * BLOCK_SIZE, SEEK_SET);
	fwrite(&directories, sizeof(directories), 1, diskptr);
}

int filesystem::getInodeListBlockCount()
{
	int res = ceil((float)(INODES * sizeof(Inode)) / BLOCK_SIZE);
	return res;
}

Inode filesystem::iget(unsigned short inode_id)
{
	Inode inode;
	fseek(fr, BLOCK_SIZE * 2, SEEK_SET);
	fread(&inode, sizeof(Inode), 1, diskptr);

	return inode;
}

void filesystem::touch(unsigned short parent_inode, const char *file_name)
{
	if (strlen(file_name) >= sizeof(DirectoryItem::name))
	{
		std::cout << "The filename name is too long. \n";
		return;
	}

	//Check if there is a file with the same name, if there is an error, exit the program. If not, create an empty file
	DirectoryItem dirlist[16]; //Temporary directory list

	Inode current_inode;
	//fseek(fr, parent_inode, SEEK_SET); // parent_inode contaions the inode offset so calculate it's position on the file and read it
	//fread(&cur, sizeof(Inode), 1, fr);
}

Inode filesystem::namei(const char *path)
{
	/*
if (path name starts from root)
	working inode - root inode (algorithm iget);
else
	working inode - current directory inode (algorithm iget);
while (there is more path name)
{
	read next path name component from input;
	verify that working inode is of directory, access permissions OK;
	if (working inode is of root and component is " .. ")
		continue;
	read directory (working inode) by repeated use of algorithms
		bmap, bread and brelse;
	if (component matches an entry in directory (working inode))
	{
		get inode number for matched component;
		release working inode (algorithm iput);
		working inode - inode of matched component (algorithm iget);
	}
	else
		return (no inode);
}
return (working inode);
*/
	Inode current_inode;
	if (path[0] == '/')
	{
		// Get root Inode
	}

	return current_inode;
}

bool filesystem::format(const char *diskfile)
{
	return false;
}

int filesystem::InodeAlloc() //Allocate i-node area function and return the inode address
{
	//Search for free inodes sequentially in the inode bitmap, and return the inode address if found. The function ends.
	if (superblock.free_inodes_num == 0)
	{
		printf("No free inode can be allocated.\n");
		return -1;
	}
	else
	{

		//Find free inodes sequentially
		int i;
		for (i = 0; i < superblock.inodelist_size; i++)
		{
			if (inode_bitmap[i] == 0) // find free inode
				break;
		}

		//Update super block
		superblock.free_inodes_num--; //free inode number -1
		fseek(fw, Superblock_StartAddr, SEEK_SET);
		fwrite(&superblock, sizeof(SuperBlock), 1, fw);

		//Update the inode bitmap
		inode_bitmap[i] = 1;
		fseek(fw, InodeBitmap_StartAddr + i, SEEK_SET);
		fwrite(&inode_bitmap[i], sizeof(bool), 1, fw);
		fflush(fw);

		return Inode_StartAddr + i * sizeof(Inode);
	}
}

int filesystem::BlockAlloc() //Disk block allocation function
{
	//Use the free block stack in the super block
	//Calculate the current stack top
	int top; //Top pointer
	if (superblock.free_blocks_num == 0)
	{ //The number of remaining free blocks is 0
		printf("No free blocks can be allocated.\n");
		return -1; //There is no free block to allocate, return -1
	}
	else
	{ //There are remaining blocks
		top = (superblock.free_blocks_num - 1);
	}
	//Remove the top of the stack
	//If it is already at the bottom of the stack, return the current block number address, which is the bottom block number of the stack, and overwrite the original stack with the new free block stack pointed to by the bottom of the stack
	int retAddr;

	if (top == 0)
	{
		retAddr = superblock.next_free_block;
		superblock.next_free_block = superblock.free_blocks[0]; //Take out the position of the next free block with free block stack, update the free block stack pointer
																//Take out the content of the corresponding free block and overwrite the original free block stack
																//Take out the next free block stack and overwrite the original
		fseek(fr, superblock.next_free_block, SEEK_SET);
		fread(superblock.free_blocks, sizeof(superblock.free_blocks), 1, fr);
		fflush(fr);

		superblock.free_blocks_num--;
	}
	else
	{										   //If it is not the bottom of the stack, the address pointed to by the top of the stack is returned, and the top pointer of the stack is -1.
		retAddr = superblock.free_blocks[top]; //Save return address
		superblock.free_blocks[top] = -1;	   //Clear top
		top--;								   //Top pointer -1
		superblock.free_blocks_num--;		   //Number of free blocks-1
	}

	//Update super block
	fseek(fw, Superblock_StartAddr, SEEK_SET);
	fwrite(&superblock, sizeof(SuperBlock), 1, fw);
	fflush(fw);

	//Update block bitmap
	block_bitmap[(retAddr - Block_StartAddr) / BLOCK_SIZE] = 1;
	fseek(fw, (retAddr - Block_StartAddr) / BLOCK_SIZE + BlockBitmap_StartAddr, SEEK_SET); //(retAddr-Block_StartAddr)/BLOCK_SIZE is the free block
	fwrite(&block_bitmap[(retAddr - Block_StartAddr) / BLOCK_SIZE], sizeof(bool), 1, fw);
	fflush(fw);

	return retAddr;
}