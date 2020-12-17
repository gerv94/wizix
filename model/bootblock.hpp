#include "../lib/shared.hpp"

struct BootBlock
{
	unsigned char data[BLOCK_SIZE] = {'*'};
};
