#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>

#include "model/filesystem.hpp"


int main(int argc, char const *argv[])
{
	filesystem* fs = new filesystem();
	fs->init();
	fs->startConsole();
	delete(fs);
}
