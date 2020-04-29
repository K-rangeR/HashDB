#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "pipeline.h"


static void create_test_data_dir();


/*
 * Test pipeline driver program
 */
int main(int argc, char *argv[])
{
	if (argc != 2) {
		printf("Usage: %s [stage_file]\n", argv[0]);
		exit(1);
	}

	create_test_data_dir();

	struct pipeline *pl = pl_init();
	if (pl == NULL) {
		printf("No memory\n");
		exit(1);
	}

	if (pl_parse_stage_file(pl, argv[1]) < 0) {
		printf("[!] Could not parse '%s', stopping...\n", argv[1]);
		pl_free(pl);
		exit(1);
	}

	pl_run(pl);

	pl_free(pl);
	exit(0);
}


static void create_test_data_dir()
{
	DIR *tdata_dir = opendir(TEST_DATA_DIR);
	if (tdata_dir) {
		closedir(tdata_dir);	
	} else if (errno == ENOENT) {
		if (mkdir(TEST_DATA_DIR, 0744) < 0) {
			printf("[!] Could not create test data directory: %s\n",
				strerror(errno));
			exit(1);
		}
	} else {
		printf("[!] Could not check for test data directory: %s\n",
			strerror(errno));
		exit(1);
	}
}
