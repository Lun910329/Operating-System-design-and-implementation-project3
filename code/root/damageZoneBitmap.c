#include <sys/param.h>
#include <sys/stat.h>

#include <ctype.h>
#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char **argv)
{
	char *mnt = "/";
	int fd = open(mnt, O_RDONLY);
	char buffer[8192];
	read(fd, buffer, 8192);
	int *p = (int*) (buffer + 512);
	for (int i = 0; i < 256; i++)
	{
		for (int j = 0; j < 32; j++)
		{
			if (p[i] & (1 << j))
			{
				putchar('0');
			}
			else
			{
				putchar('1');
			}
		}
	}
	//printf("%d\n", *p);
	close(fd);
	putchar('\n');
	return 0;
}
