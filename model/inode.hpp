#include <ctime>

enum INODE_TYPE
{
	REGULAR,
	DIRECTORY,
	CHARACTER,
	SPECIAL,
	FIFO
};


struct Inode
{
	// ::::::::::::::::::::::::::::::::::::::::::::::::

	unsigned short own_usr;
	unsigned short own_grp;
	INODE_TYPE type;
	unsigned short permissions;
	time_t modify_time;
	time_t access_time;
	time_t inode_time;
	unsigned short links;
	unsigned short content[13];
	unsigned short size;
};

/*

::::::: Documentation :::::::

* File owner identifier. Ownership is divided between an individual owner and a
"group" owner and defines the set of users who have access rights to a file. The
superuser has access rights to all files in the system.
- own_id		4 bytes

* File type. Files may be of type regular, directory, character or block special, or
FIFO (pipes). 
- type			4 bytes

* File access permissions. The system protects files according to three classes:
the owner and the group owner of the file, and other users; each class has access
rights to read, write and execute the file, which can be set individually. Because
directories cannot be executed, execution permission for a directory gives the
right to search the directory for a file name. 
- permissions	4 bytes

* File access times, giving the time the file was last modified, when it was last
accessed, and when the inode was last modified. 
- modify_time	4 bytes
- access_time	4 bytes
- inode_time	4 bytes

* Number of links to the file, representing the number of names the file has in the
directory hierarchy. 
- links: int	4 bytes

* Table of contents for the disk addresses of data in a file. Although users treat
the data in a file as a logical stream of bytes, the kernel saves the data in
discontiguous disk blocks. The inode identifies the disk !blocks that contain the
file's data. 
- content: int[13] 40 bytes

* File size. Data in a file is addressable by the number of bytes from the
beginning of the file, starting from byte offset 0, and the file size is 1 greater
than the highest byte offset of data in the file. For example, if a user creates a
file and writes only 1 byte of data at byte offset 1000 in the file, the size of the
file is 1001 bytes. 
- size: int		4 bytes

- total: 72



10 direct blocks with 1K bytes each -						10K bytes
1 indirect block with 256 direct blocks -					256K bytes
1 double indirect block with 256 indirect blocks -			64M bytes
1 triple indirect block with 256 double indirect blocks -	16G bytes





*/