struct inode
{
	/**
	10 direct blocks with lK bytes each - lOK bytes
	l indirect block with 256 direct blocks - 256K bytes
	l double indirect block with 256 indirect blocks - 64M bytes
	l triple indirect block with 256 double indirect blocks - 16G bytes
	**/
	// Owner UID = int = 4bytes
	// Group UID = int = 4bytes
	// permissions = 9 bits
	// mode = 
	// timestamp = 
	__UINT32_TYPE__ directs[12]; // 12 * 4bytes = 48 bytes
};