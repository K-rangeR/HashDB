#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pipeline.h"

int main(int argc, char *argv[])
{
	if (argc != 2) {
		printf("Usage: %s [stage_file]\n", argv[0]);
		exit(1);
	}

	struct pipeline *pl = pl_init();
	if (pl == NULL) {
		printf("No memory\n");
		exit(1);
	}

	if (pl_parse_stage_file(pl, argv[1]) < 0) {
		printf("[!] Could not parse '%s': %s\n", argv[1],
			strerror(errno));
		pl_free(pl);
		exit(1);
	}

	pl_run(pl);

	pl_free(pl);
	exit(0);
}
