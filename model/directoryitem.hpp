struct DirectoryItem
{
	unsigned short inode = 0;
	char name[14] = {0};
};

/*

::::::: Documentation :::::::

Each component except the last must be the name of
a directory, but the last component may be a non-directory file. UNIX System V
restricts component names to a maximum of 14 characters; with a 2 byte entry for
the inode number, the size of a directory entry is 16 bytes. 

*/