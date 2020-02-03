#include <stdlib.h>
#include <stdio.h>
#include "segment.h"

/*
 * Allocates and returns a pointer to a segment_file struct. Note, this
 * does not create a segment file to back this struct, call segf_create_file
 * for that. For that reason some fields are set to default values:
 *	- size to 0
 *	- seg_fd to -1
 *	- next to null
 *
 * Params:
 *	name => name of the segment file to use when its created
 *
 * Returns:
 *	pointer to a segment_file struct if successful, NULL otherwise
 *	(caller must free this struct by using segf_free)
 */
struct segment_file *segf_init(char *name)
{
	struct segment_file *seg;

	if ((seg = malloc(sizeof(struct segment_file))) == NULL)
		return NULL;

	seg->size = 0;
	seg->name = name;
	seg->seg_fd = -1;
	if ((seg->table = memtable_init()) == NULL) {
		free(seg);
		return NULL;
	}
	seg->next = NULL;

	return seg;
}

/*
 * Deallocates the memory used by the give segment_file struct, including its
 * memtable.
 *
 * Params:
 *	seg => segment_file struct to free
 *
 * Returns:
 *	void
 */
void segf_free(struct segment_file *seg)
{
	memtable_free(seg->table);
	free(seg);
}
