/*
 * Uses the segment file IO functions to create permanent segment files
 * that can be used by all the tests in check_segment and check_hashDB.
 *
 * Should only have to execute this program once for every version of the
 * segf_append function.
 *
 * The functions used are assumed to be correct.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "../src/segment.h"
#include "data.h"

static struct segment_file *init_new_seg(int idx)
{
	struct segment_file *seg;

	if ((seg = segf_init(test_file_namesxl[idx])) == NULL) {
		printf("ERROR: init: could not create segment struct:");
		printf("%s\n", strerror(errno));
		exit(1);
	}

	if (segf_create_file(seg) < 0) {
		printf("ERROR: create: could not open segment struct:");
		printf("%s\n", strerror(errno));
		exit(1);
	}
	
	return seg;
}

int main(void)
{
	struct segment_file *seg = NULL;
	int idx = 0;

	for (int i = 0; i < TOTAL_KVXL_PAIRS; ++i) {
		if (i % KV_PAIRS_PER_FILE == 0) {
			seg = init_new_seg(idx);
			idx += 1;
		}

		if (segf_append(seg, tdxl[i].key, 
				tdxl[i].val, TOMBSTONE_INS) < 0) {
			printf("ERROR: append failed: %s\n", 
				strerror(errno));
			exit(1);
		}
	}

	for (int i = 0; i < TOTAL_KVXL_PAIRS; ++i) {
		if (i % KV_PAIRS_PER_FILE == 0) {
			if (segf_remove_pair(seg, tdxl[i].key) < 0) {
				printf("ERROR: remove failed:%s\n", 
					strerror(errno));
				exit(1);
			}
		}
	}

	exit(0);
}
