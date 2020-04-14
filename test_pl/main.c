#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
	if (argc != 2) {
		printf("Usage: %s [stage_file]\n", argv[0]);
		exit(1);
	}

	exit(0);
}
