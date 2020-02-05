#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
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

/*
 * Adds the key offset pair to the given segment files memtable.
 *
 * Params:
 *	seg => segment file to update
 *	key => data's key in the segment file
 *	offset => data's offset in the segment file
 *
 * Returns:
 *	-1 if there is an error adding the pair, 0 otherwise
 */
int segf_update_memtable(struct segment_file *seg, int key, unsigned int offset)
{
	if (memtable_write(seg->table, key, offset) < 0)
		return -1;
	return 0;
}

/*
 * Reads the offset from the segment file memtable at the given key.
 *
 * Params:
 *	seg => container for the segment files memtable
 *	key => key for the data of interest
 *	offset => address of where to store the data's offset if found
 *
 * Returns:
 *	1 if the key and offset were found, 0 otherwise (offset not changed)
 */
int segf_read_memtable(struct segment_file *seg, int key, unsigned int *offset)
{
	return memtable_read(seg->table, key, offset);
}

/*
 * Creates a new segment file with the name seg->name. The file is opened
 * for read and write operations. The file descriptor is set in seg->seg_fd.
 *
 * Params:
 *	seg => pointer to a segment_file struct that contains the name of the
 *	       segment file to create
 *
 * Returns:
 *	0 if successful, -1 if there was an error (check errno)
 */
int segf_create_file(struct segment_file *seg)
{
	int fd;

	if ((fd = open(seg->name, O_CREAT|O_TRUNC|O_RDWR, 0664)) < 0)
		return -1;

	seg->seg_fd = fd;
	return 0;	
}

/*
 * Closes and deletes the segment file backing the given segment_file struct. 
 * Also sets the size field in the struct to 0 and seg_fd back to -1.
 *
 * Params:
 *	seg => pointer to a segment_file struct that contains the name of the
 *	       file to delete and the file descriptor to close
 *
 * Returns:
 *	0 if successful, -1 if there was an error (check errno)
 */
int segf_delete_file(struct segment_file *seg)
{
	close(seg->seg_fd);

	if (remove(seg->name) < 0)
		return -1;
	
	seg->seg_fd = -1;
	seg->size = 0;
	return 0;
}

/*
 * Puts s1 before s2 in the linked list
 *
 * Params:
 *	s1 => new predecessor of s2
 *	s2 => new successor of s1
 *
 * Returns:
 *	void
 */
void segf_link_before(struct segment_file *s1, struct segment_file *s2)
{
	if (s1 == NULL) {
		fprintf(stderr, "segf_link_before: s1 was NULL\n");
		exit(1);
	}
	s1->next = s2;
}

/*
 * Removes seg from the given list. Does not free seg.
 *
 * Params:
 *	head => start of the linked list
 *	seg => segment_file to remove from the linked list
 *
 * Returns:
 *	void
 */
void segf_unlink(struct segment_file *head, struct segment_file *seg)
{
	struct segment_file *prev;

	if (head == seg) {
		seg->next = NULL;
		return;
	}

	prev = head;
	while (prev && prev->next != seg)
		prev = prev->next;
	
	prev->next = seg->next;
	seg->next = NULL;
}
