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

void filesystem::startConsole()
{
	char command[64];
	unsigned int current_inode = EXT4_ROOT_INO;
	while (true)
	{
		std::cout << "root@wizix:/$ "; // TODO: print current directory
		std::cin >> command;

		if (strcmp(command, "help") == 0)
		{
			printf("Help command\n");
		}
		else if (strcmp(command, "exit") == 0)
		{
			printf("Bye\n");
			break;
		}
		else if (strcmp(command, "clear") == 0)
		{
			// CSI[2J clears screen, CSI[H moves the cursor to top-left corner
			std::cout << "\x1B[2J\x1B[H";
		}
		else if (strcmp(command, "ls") == 0)
		{
			ls(iget(current_inode));
		}
		else if (strcmp(command, "mkdir") == 0)
		{
			std::cin >> command;
			mkdir(current_inode, command);
		}
		else if (strcmp(command, "touch") == 0)
		{
			std::cin >> command;
			touch(iget(current_inode), command);
		}
		else if (strcmp(command, "vi") == 0)
		{
			std::cin >> command;
			vi(iget(current_inode), command);
		}
		else if (strcmp(command, "cat") == 0)
		{
			std::cin >> command;
			cat(iget(current_inode), command);
		}
		else if (strcmp(command, "rm") == 0)
		{
			std::cin >> command;
			rm(iget(current_inode), command);
		}
		else if (strcmp(command, "rmdir") == 0)
		{
			std::cin >> command;
			rmdir(iget(current_inode), command);
		}
		else if (strcmp(command, "cd") == 0)
		{
			std::cin >> command;
			unsigned short inode_id = namei(iget(current_inode), command);

			//TODO: validate is directory
			if (inode_id == 0)
			{
				std::cout << "wizix: cd: " << command << ": No such file or directory\n";
			}
			else
			{
				current_inode = inode_id;
			}
		}
		else
		{
			std::cout << "wizix: " << command << ": command not found\n";
		}
	}
}

void filesystem::mountDisk()
{
	LOG("Mounting disk");

	diskptr = fopen(DISK_FILE, "rb+");

	fread(bootblock.data, BLOCK_SIZE, 1, diskptr);

	fseek(diskptr, BLOCK_SIZE, SEEK_SET);
	fread(&superblock, sizeof(SuperBlock), 1, diskptr);

	LOG("File System size: " << superblock.filesystem_size << " bytes");
	LOG("Free Blocks: " << superblock.free_blocks_num);
	LOG("Next first block: " << superblock.next_free_block);

	LOG("Inode List size: " << superblock.inodelist_size);
	LOG("Free Inodes: " << superblock.free_inodes_num);
	LOG("Next first inode: " << superblock.next_free_inode);

	fclose(diskptr);

	LOG("Finished mounting disk");
}

void filesystem::createDisk(const char *disk_file)
{
	LOG("Creating disk");

	char buffer[BLOCK_SIZE] = {0};
	diskptr = fopen(disk_file, "wb+");
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

	for (int i = 0; i < BLOCKS; i++)
	{
		superblock.free_blocks[i] = 0;
	}

	for (int i = 0; i < INODES; i++)
	{
		superblock.free_inodes[i] = 0;
	}

	// Storing BootBlock
	for (int i = 0; i < BLOCK_SIZE; i++)
	{
		bootblock.data[i] = '*';
	}
	fseek(diskptr, 0, SEEK_SET);
	fwrite(&bootblock, sizeof(BootBlock), 1, diskptr);

	// Storing Inode list after SuperBlock
	createRootDir();

	// Remove first 3 inodes from free inode, including root
	superblock.free_inodes[0] = 1;
	superblock.free_inodes[EXT4_BAD_INO] = 1;
	superblock.free_inodes[EXT4_ROOT_INO] = 1;
	superblock.next_free_inode = EXT4_ROOT_INO + 1;
	superblock.free_inodes_num = superblock.free_inodes_num - 3;

	// Remove root dir block from free_blocks
	superblock.free_blocks[superblock.next_free_block] = 1;
	superblock.next_free_block = superblock.next_free_block + 1;
	superblock.free_blocks_num = superblock.free_blocks_num - 1;

	// Storing SuperBlock after BootBlock
	fseek(diskptr, BLOCK_SIZE, SEEK_SET);
	fwrite(&superblock, sizeof(SuperBlock), 1, diskptr);

	fclose(diskptr);

	LOG("File System size: " << superblock.filesystem_size << " bytes");
	LOG("Free Blocks: " << superblock.free_blocks_num);
	LOG("Next first block: " << superblock.next_free_block);

	LOG("Inode List size: " << superblock.inodelist_size);
	LOG("Free Inodes: " << superblock.free_inodes_num);
	LOG("Next first inode: " << superblock.next_free_inode);

	LOG("Inode List Blocks: " << inodeListBlocks << " blocks");

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
	root_inode.permissions = 0755; // d rwx r-x r-x
	root_inode.modify_time = now;
	root_inode.access_time = now;
	root_inode.inode_time = now;
	root_inode.links = 1;
	root_inode.content[0] = superblock.next_free_block;
	root_inode.size = BLOCK_SIZE;

	// store dir content
	DirectoryItem directories[MAX_DIR];
	directories[0].inode = EXT4_ROOT_INO;
	sprintf(directories[0].name, ".");

	directories[1].inode = EXT4_ROOT_INO;
	sprintf(directories[1].name, "..");

	//Store content on next free_block
	fseek(diskptr, root_inode.content[0] * BLOCK_SIZE, SEEK_SET);
	fwrite(&directories, sizeof(directories), 1, diskptr);

	//Store inode
	fseek(diskptr, (BLOCK_SIZE * 2) + (sizeof(Inode) * EXT4_ROOT_INO), SEEK_SET);
	fwrite(&root_inode, sizeof(Inode), 1, diskptr);

	LOG("ROOT dir stored on: " << (BLOCK_SIZE * 2) + (sizeof(Inode) * EXT4_ROOT_INO));
	LOG("ROOT dir stored on: " << (BLOCK_SIZE * 2) + (sizeof(Inode) * EXT4_ROOT_INO));
}

int filesystem::getInodeListBlockCount()
{
	int res = ceil((float)(INODES * sizeof(Inode)) / BLOCK_SIZE);
	return res;
}

Inode filesystem::iget(unsigned short inode_id)
{
	LOG("starting iget getting inode: " << inode_id);
	Inode inode;

	diskptr = fopen(DISK_FILE, "rb+");
	fseek(diskptr, (BLOCK_SIZE * 2) + (sizeof(Inode) * inode_id), SEEK_SET);
	fread(&inode, sizeof(Inode), 1, diskptr);
	fclose(diskptr);

	LOG("finished iget");

	return inode;
}

void filesystem::touch(Inode parent_inode, const char *file_name)
{
	unsigned int file_location;
	if (strlen(file_name) >= sizeof(DirectoryItem::name))
	{
		std::cout << "The filename name is too long. \n";
		return;
	}

	DirectoryItem dirlist[MAX_DIR]; //Temporary directory list
	diskptr = fopen(DISK_FILE, "rb+");
	fseek(diskptr, parent_inode.content[0] * BLOCK_SIZE, SEEK_SET);
	fread(&dirlist, sizeof(dirlist), 1, diskptr);
	fclose(diskptr);

	// Verify if a file or directory already exists with that name
	for (int i = 0; i < MAX_DIR; i++)
	{
		if (dirlist[i].inode != 0)
		{
			//Inode tmp = iget(dirlist[i].inode);
			if (strcmp(dirlist[i].name, file_name) == 0)
			{
				std::cout << "wizix: touch " << file_name << ": File already exists\n";
				return;
			}
		}
		else
		{
			file_location = i;
			break;
		}
	}

	// Decrement free inodes
	unsigned int inode_id = superblock.next_free_inode;
	unsigned int block_id = superblock.next_free_block;
	if (inode_id > 0 && block_id > 0)
	{

		superblock.free_inodes[superblock.next_free_inode] = 1;
		superblock.free_blocks[superblock.next_free_block] = 1;
		superblock.free_inodes_num--;
		superblock.free_blocks_num--;
		superblock.next_free_inode = getNextFreeInode();
		superblock.next_free_block = getNextFreeBlock();

		Inode file_inode;
		time_t now = time(0);
		file_inode.own_usr = 0; // meaning root
		file_inode.own_grp = 0; // meaning root
		file_inode.type = REGULAR;
		file_inode.permissions = 0664; // TODO:
		file_inode.modify_time = now;
		file_inode.access_time = now;
		file_inode.inode_time = now;
		file_inode.links = 1;
		file_inode.content[0] = block_id;
		file_inode.size = BLOCK_SIZE;

		diskptr = fopen(DISK_FILE, "rb+");
		//Store new file inode
		fseek(diskptr, (BLOCK_SIZE * 2) + (sizeof(Inode) * inode_id), SEEK_SET);
		fwrite(&file_inode, sizeof(Inode), 1, diskptr);

		//Update parent inode dir items
		dirlist[file_location].inode = inode_id;
		sprintf(dirlist[file_location].name, file_name);

		for (int i = 0; i < MAX_DIR; i++)
		{
			LOG("Parent dirlist[" << i << "] inode: " << dirlist[i].inode << " name: " << dirlist[i].name);
		}

		fseek(diskptr, parent_inode.content[0] * BLOCK_SIZE, SEEK_SET);
		fwrite(&dirlist, sizeof(dirlist), 1, diskptr);

		fclose(diskptr);

		updateSuperBlock();

		LOG("Creating file on inode: " << inode_id);
		LOG("Creating file on block: " << block_id);
	}
	else
	{
		std::cout << "wizix: not enough space\n";
	}
}

void filesystem::rm(Inode parent_inode, const char *file_name)
{
	DirectoryItem dirlist[MAX_DIR]; //Temporary directory list
	diskptr = fopen(DISK_FILE, "rb+");
	fseek(diskptr, parent_inode.content[0] * BLOCK_SIZE, SEEK_SET);
	fread(&dirlist, sizeof(dirlist), 1, diskptr);
	fclose(diskptr);

	// Verify if a file exists
	for (int i = 0; i < MAX_DIR; i++)
	{
		if (dirlist[i].inode != 0)
		{
			if (strcmp(dirlist[i].name, file_name) == 0)
			{
				Inode tmp = iget(dirlist[i].inode);

				if (tmp.type == DIRECTORY)
				{
					std::cout << "rm: cannot remove '" << file_name << "': Is a directory\n";
					return;
				}
				else if (tmp.type == REGULAR)
				{
					LOG("Removing file at inode: " << dirlist[i].inode);
					superblock.free_inodes[dirlist[i].inode] = 0;

					for (int i = 0; i < 10; i++)
					{
						if (tmp.content[i] > 0)
						{
							superblock.free_blocks[tmp.content[i]] = 0;
							superblock.free_blocks_num++;
						}
					}

					superblock.free_inodes_num++;

					superblock.next_free_inode = getNextFreeInode();
					superblock.next_free_block = getNextFreeBlock();
					updateSuperBlock();

					dirlist[i].inode = 0; // The directory item only needs to set to 0

					diskptr = fopen(DISK_FILE, "rb+");
					fseek(diskptr, parent_inode.content[0] * BLOCK_SIZE, SEEK_SET);
					fwrite(&dirlist, sizeof(dirlist), 1, diskptr);

					fclose(diskptr);
					return;
				}
			}
		}
	}
}

void filesystem::rmdir(Inode parent_inode, const char *file_name)
{
	DirectoryItem dirlist[MAX_DIR]; //Temporary directory list
	diskptr = fopen(DISK_FILE, "rb+");
	fseek(diskptr, parent_inode.content[0] * BLOCK_SIZE, SEEK_SET);
	fread(&dirlist, sizeof(dirlist), 1, diskptr);
	fclose(diskptr);

	// Verify if a file exists
	for (int i = 0; i < MAX_DIR; i++)
	{
		if (dirlist[i].inode != 0)
		{
			if (strcmp(dirlist[i].name, file_name) == 0)
			{
				Inode tmp = iget(dirlist[i].inode);

				if (tmp.type == DIRECTORY)
				{

					DirectoryItem tmp_dirlist[MAX_DIR]; //Temporary directory list
					diskptr = fopen(DISK_FILE, "rb+");
					fseek(diskptr, tmp.content[0] * BLOCK_SIZE, SEEK_SET);
					fread(&tmp_dirlist, sizeof(tmp_dirlist), 1, diskptr);
					fclose(diskptr);
					for (int j = 0; j < MAX_DIR; j++)
					{
						if (tmp_dirlist[j].inode != 0)
						{
							if (tmp_dirlist[j].inode != dirlist[i].inode && tmp_dirlist[j].inode != dirlist[0].inode)
							{
								// Verify if an inode points other than this directory or parent directory
								std::cout << "rm: cannot remove '" << file_name << "': Is not an empty directory\n";
								return;
							}
						}
					}

					LOG("Removing directory at inode: " << dirlist[i].inode);
					superblock.free_inodes[dirlist[i].inode] = 0;
					superblock.free_blocks[tmp.content[0]] = 0; //TODO: iterate all blocks if needed
					superblock.free_inodes_num++;
					superblock.free_blocks_num++;
					superblock.next_free_inode = getNextFreeInode();
					superblock.next_free_block = getNextFreeBlock();
					updateSuperBlock();

					dirlist[i].inode = 0; // The directory item only needs to set to 0

					diskptr = fopen(DISK_FILE, "rb+");
					fseek(diskptr, parent_inode.content[0] * BLOCK_SIZE, SEEK_SET);
					fwrite(&dirlist, sizeof(dirlist), 1, diskptr);

					fclose(diskptr);
					return;
				}
				else if (tmp.type == REGULAR)
				{
					std::cout << "rm: cannot remove '" << file_name << "': Is not a directory\n";
					return;
				}
			}
		}
	}
}

void filesystem::mkdir(unsigned short parent_inode_id, const char *file_name)
{
	Inode parent_inode = iget(parent_inode_id);
	unsigned int dir_location;
	if (strlen(file_name) >= sizeof(DirectoryItem::name))
	{
		std::cout << "The directory name is too long. \n";
		return;
	}

	LOG("Next free inode: " << superblock.next_free_inode);

	DirectoryItem dirlist[MAX_DIR]; //Temporary directory list
	diskptr = fopen(DISK_FILE, "rb+");
	fseek(diskptr, parent_inode.content[0] * BLOCK_SIZE, SEEK_SET);
	fread(&dirlist, sizeof(dirlist), 1, diskptr);
	fclose(diskptr);

	// Verify if a file or directory already exists with that name
	for (int i = 0; i < MAX_DIR; i++)
	{
		if (dirlist[i].inode != 0)
		{
			//Inode tmp = iget(dirlist[i].inode);
			if (strcmp(dirlist[i].name, file_name) == 0)
			{
				std::cout << "wizix: mkdir " << file_name << ": File already exists\n";
				return;
			}
		}
		else
		{
			dir_location = i;
			break;
		}
	}

	// Decrement free inodes
	unsigned int inode_id = superblock.next_free_inode;
	unsigned int block_id = superblock.next_free_block;
	if (inode_id > 0 && block_id > 0)
	{

		superblock.free_inodes[superblock.next_free_inode] = 1;
		superblock.free_blocks[superblock.next_free_block] = 1;
		superblock.free_inodes_num--;
		superblock.free_blocks_num--;
		superblock.next_free_inode = getNextFreeInode();
		superblock.next_free_block = getNextFreeBlock();

		Inode dir_inode;
		time_t now = time(0);
		dir_inode.own_usr = 0; // meaning root
		dir_inode.own_grp = 0; // meaning root
		dir_inode.type = DIRECTORY;
		dir_inode.permissions = 0755; // d rwx r-x r-x
		dir_inode.modify_time = now;
		dir_inode.access_time = now;
		dir_inode.inode_time = now;
		dir_inode.links = 1;
		dir_inode.content[0] = block_id;
		dir_inode.size = BLOCK_SIZE;

		// Directory content
		DirectoryItem directories[MAX_DIR];
		directories[0].inode = inode_id;
		sprintf(directories[0].name, ".");

		directories[1].inode = parent_inode_id;
		sprintf(directories[1].name, "..");

		diskptr = fopen(DISK_FILE, "rb+");
		//Store content on next free_block
		fseek(diskptr, dir_inode.content[0] * BLOCK_SIZE, SEEK_SET);
		fwrite(&directories, sizeof(directories), 1, diskptr);

		//Store new directory inode
		fseek(diskptr, (BLOCK_SIZE * 2) + (sizeof(Inode) * inode_id), SEEK_SET);
		fwrite(&dir_inode, sizeof(Inode), 1, diskptr);

		LOG("NEW dir stored on: " << (BLOCK_SIZE * 2) + (sizeof(Inode) * inode_id));

		//Update parent inode dir items
		dirlist[dir_location].inode = inode_id;
		sprintf(dirlist[dir_location].name, file_name);

		for (int i = 0; i < MAX_DIR; i++)
		{
			LOG("Parent dirlist[" << i << "] inode: " << dirlist[i].inode << " name: " << dirlist[i].name);
		}

		fseek(diskptr, parent_inode.content[0] * BLOCK_SIZE, SEEK_SET);
		fwrite(&dirlist, sizeof(dirlist), 1, diskptr);

		fclose(diskptr);

		updateSuperBlock();

		LOG("Creating directory on inode: " << inode_id);
		LOG("Creating directory on block: " << block_id);
	}
	else
	{
		std::cout << "wizix: not enough space\n";
	}
}

void filesystem::vi(Inode parent_inode, const char *file_name)
{
	// search for the file being created
	DirectoryItem dirlist[MAX_DIR]; //Temporary directory list
	diskptr = fopen(DISK_FILE, "rb+");
	fseek(diskptr, parent_inode.content[0] * BLOCK_SIZE, SEEK_SET);
	fread(&dirlist, sizeof(dirlist), 1, diskptr);
	fclose(diskptr);

	unsigned short inode_id = 0;
	Inode inode;
	// Verify if a file exists
	for (int i = 0; i < MAX_DIR; i++)
	{
		if (dirlist[i].inode != 0)
		{
			if (strcmp(dirlist[i].name, file_name) == 0)
			{
				inode_id = dirlist[i].inode;
			}
		}
	}
	if (inode_id > 0)
	{
		inode = iget(inode_id);
		if (inode.type == DIRECTORY)
		{
			std::cout << "vi: cannot open '" << file_name << "': Is a directory\n";
			return;
		}
	}
	else
	{
		std::cout << "vi: cannot open '" << file_name << "': No such file exists\n";
		return;
	}

	LOG("Starting mini-VI");
	std::cout << "Welcome to the mini-VI (a per line editor), type (in a new line) :q to exit and :wq to save and exit\n";
	std::cout.flush();
	std::string s;
	do
	{
		std::string tmp_s;
		getline(std::cin, tmp_s);
		if (tmp_s.compare(":q") == 0)
		{
			s = "";
			return;
		}
		if (tmp_s.compare(":wq") == 0)
		{
			break;
		}
		s += (tmp_s + "\n");
	} while (!std::cin.eof());

	// Remove garbage from imput
	s.pop_back();
	s = s.substr(1);
	std::cin.clear();

	LOG("Storing: " << s.size() << " bytes");
	LOG(s);

	char buffer[BLOCK_SIZE];
	if (s.size() <= BLOCK_SIZE)
	{
		strcpy(buffer, s.c_str());

		//Store content on inode first datablock
		diskptr = fopen(DISK_FILE, "rb+");
		fseek(diskptr, inode.content[0] * BLOCK_SIZE, SEEK_SET);
		fwrite(&buffer, BLOCK_SIZE, 1, diskptr);
		fclose(diskptr);
	}
	else if (s.size() <= BLOCK_SIZE * 10)
	{
		LOG("Storing on the direct blocks");
		int blocks = ceil(((float)s.size()) / BLOCK_SIZE);
		LOG("Needs " << blocks << " blocks to store this file");

		if (blocks < superblock.free_blocks_num)
		{
			for (int i = 0; i < blocks; i++)
			{
				LOG("substring(" << BLOCK_SIZE * i << ", " << BLOCK_SIZE << ")");

				std::string substring = s.substr(BLOCK_SIZE * i, BLOCK_SIZE);

				LOG("Storing: " << substring.size());
				LOG(substring);

				memcpy(buffer, substring.c_str(), BLOCK_SIZE);

				if (i > 0)
				{ //Update superblock
					unsigned short block_id = superblock.next_free_block;
					superblock.free_blocks[superblock.next_free_block] = 1;
					superblock.free_blocks_num--;
					superblock.next_free_block = getNextFreeBlock();

					updateSuperBlock();

					inode.content[i] = block_id;

					// Store data block
					diskptr = fopen(DISK_FILE, "rb+");
					fseek(diskptr, inode.content[i] * BLOCK_SIZE, SEEK_SET);
					fwrite(&buffer, BLOCK_SIZE, 1, diskptr);
					fclose(diskptr);
				}
				else
				{
					// Store data block
					diskptr = fopen(DISK_FILE, "rb+");
					fseek(diskptr, inode.content[0] * BLOCK_SIZE, SEEK_SET);
					fwrite(&buffer, BLOCK_SIZE, 1, diskptr);
					fclose(diskptr);
				}
			}
			LOG("Updating inode: " << inode_id);
			time_t now = time(0);
			inode.modify_time = now;
			inode.size = blocks * BLOCK_SIZE;
			// Update inode content
			diskptr = fopen(DISK_FILE, "rb+");
			fseek(diskptr, (BLOCK_SIZE * 2) + (sizeof(Inode) * inode_id), SEEK_SET);
			fwrite(&inode, sizeof(Inode), 1, diskptr);
			fclose(diskptr);
		}
		else
		{
			std::cout << "wizix: vi " << file_name << ": There is not enough espace to store this file\n";
		}
	}
}

void filesystem::cat(Inode parent_inode, const char *file_name)
{
	// search for the file being created
	DirectoryItem dirlist[MAX_DIR]; //Temporary directory list
	diskptr = fopen(DISK_FILE, "rb+");
	fseek(diskptr, parent_inode.content[0] * BLOCK_SIZE, SEEK_SET);
	fread(&dirlist, sizeof(dirlist), 1, diskptr);
	fclose(diskptr);

	unsigned short inode_id = 0;
	Inode inode;
	// Verify if a file exists
	for (int i = 0; i < MAX_DIR; i++)
	{
		if (dirlist[i].inode != 0)
		{
			if (strcmp(dirlist[i].name, file_name) == 0)
			{
				inode_id = dirlist[i].inode;
			}
		}
	}
	if (inode_id > 0)
	{
		inode = iget(inode_id);
		if (inode.type == DIRECTORY)
		{
			std::cout << "cat: cannot open '" << file_name << "': Is a directory\n";
			return;
		}
	}
	else
	{
		std::cout << "cat: cannot open '" << file_name << "': No such file exists\n";
		return;
	}

	char buffer[BLOCK_SIZE];
	//Iterate first direct blocks
	for (int i = 0; i < 10; i++)
	{
		if (inode.content[i] > 0)
		{
			diskptr = fopen(DISK_FILE, "rb+");
			fseek(diskptr, inode.content[i] * BLOCK_SIZE, SEEK_SET);
			fread(&buffer, BLOCK_SIZE, 1, diskptr);
			fclose(diskptr);
			std::cout << buffer;
		}
		else
		{
			break;
		}
	}
	std::cout << "\n";
}

unsigned short filesystem::getNextFreeInode()
{
	for (int i = EXT4_ROOT_INO + 1; i < INODES; i++)
	{
		if (superblock.free_inodes[i] == 0)
		{
			return i;
		}
	}
	return 0;
}

unsigned short filesystem::getNextFreeBlock()
{
	int inodeblocks = getInodeListBlockCount();
	for (int i = (2 + inodeblocks + 1); i < INODES; i++)
	{
		if (superblock.free_blocks[i] == 0)
		{
			return i;
		}
	}
	return 0;
}

void filesystem::ls(Inode inode)
{
	LOG("Inode size: " << inode.size);
	switch (inode.type)
	{
	case REGULAR:
	{
		LOG("Is a regular file");
		break;
	}
	case DIRECTORY:
	{

		DirectoryItem dirlist[MAX_DIR];

		diskptr = fopen(DISK_FILE, "rb+");
		LOG("Is a directory stored at block: " << inode.content[0]);
		fseek(diskptr, inode.content[0] * BLOCK_SIZE, SEEK_SET);
		fread(&dirlist, sizeof(dirlist), 1, diskptr);
		fclose(diskptr);

		for (int i = 0; i < MAX_DIR; i++)
		{
			if (dirlist[i].inode != 0)
			{
				Inode tmp = iget(dirlist[i].inode);
				std::cout << dirlist[i].inode << "\t";
				if (tmp.type == DIRECTORY)
				{
					std::cout << "d";
				}
				else
				{
					std::cout << "-";
				}

				tm *ptr; //Storage time
				ptr = gmtime(&tmp.modify_time);

				int t = 8;
				while (t >= 0)
				{
					if (((tmp.permissions >> t) & 1) == 1)
					{
						if (t % 3 == 2)
							std::cout << "r";
						if (t % 3 == 1)
							std::cout << "w";
						if (t % 3 == 0)
							std::cout << "x";
					}
					else
					{
						std::cout << "-";
					}
					t--;
				}
				std::cout << " ";

				//
				std::cout << tmp.links << " ";
				//std::cout << tmp.own_usr << " "; //TODO: get user name from a file
				std::cout << "root"
						  << " ";
				//std::cout << tmp.own_grp << "\t"; //TODO: get group name from a file
				std::cout << "root"
						  << "\t";

				std::cout << tmp.size << "\t";
				printf("%d.%d.%d %02d:%02d:%02d ", 1900 + ptr->tm_year, ptr->tm_mon + 1, ptr->tm_mday, (8 + ptr->tm_hour) % 24, ptr->tm_min, ptr->tm_sec); //last modification time

				std::cout << dirlist[i].name << "\n";
			}
			else
			{
				LOG("Parent dirlist[" << i << "] inode: " << dirlist[i].inode << " name: " << dirlist[i].name);
			}
		}
	}
	break;
	case CHARACTER:
		//TODO:
		std::cout << "Currently not supported\n";
		break;
	case SPECIAL:
		//TODO:
		std::cout << "Currently not supported\n";
		break;
	case FIFO:
		//TODO:
		std::cout << "Currently not supported\n";
		break;
	}
}

unsigned short filesystem::namei(Inode parent_inode, const char *path)
{
	LOG("Searching directory: " << path);
	//TODO: support full path
	DirectoryItem dirlist[MAX_DIR];

	diskptr = fopen(DISK_FILE, "rb+");
	fseek(diskptr, parent_inode.content[0] * BLOCK_SIZE, SEEK_SET);
	fread(&dirlist, sizeof(dirlist), 1, diskptr);
	fclose(diskptr);
	for (int i = 0; i < MAX_DIR; i++)
	{
		if (strcmp(dirlist[i].name, path) == 0)
		{
			LOG("Changing directory to: " << dirlist[i].name);
			return dirlist[i].inode;
		}
	}
	return 0;
}

void filesystem::updateSuperBlock()
{
	// Updating SuperBlock
	diskptr = fopen(DISK_FILE, "rb+");

	fseek(diskptr, BLOCK_SIZE, SEEK_SET);
	fwrite(&superblock, sizeof(SuperBlock), 1, diskptr);

	fclose(diskptr);
}