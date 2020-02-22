#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
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
 *	name => name of the segment file to use when its created, this is
 *      must be allocated on the heap because segf_free will call free(..)
 *      to deallocate the name
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
	free(seg->name);
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
 * Opens the segment file identified by seg->name. Sets the given segment
 * file structs seg_fd field to the return file descriptor.
 *
 * Params:
 *	seg => segment file to open and assign the file descriptor
 *
 * Returns:
 *	-1 if the file can't be opened, 0 if successful
 */
int segf_open_file(struct segment_file *seg)
{
	int fd;

	if ((fd = open(seg->name, O_RDWR)) < 0)
		return -1;

	seg->seg_fd = fd;
	return 0;
}

/*
 * Closes the given segment file. Does not free the segment file struct, but
 * sets seg_fd = -1.
 *
 * Params:
 *	seg => pointer to the segment file struct to close
 *
 * Returns:
 *	void
 */
void segf_close_file(struct segment_file *seg)
{
	close(seg->seg_fd);
	seg->seg_fd = -1;
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
 * Reads the segment file associated with the given segment file struct
 * and repopulate its memtable with all keys and their value offsets.
 *
 * Params:
 *	seg => segment file struct to repopulate
 *
 * Returns:
 *	-1 if there is an error (check errno), 0 otherwise
 */
int segf_repop_memtable(struct segment_file *seg)
{
	unsigned int offset;
	int key, key_len, val_len, n, pair_deleted;
	char tombstone;

	// seek to the front of the file
	if (lseek(seg->seg_fd, 0, SEEK_SET) < 0)
		return -1;

	tombstone = 0;
	pair_deleted = 0;
	while (1) {
		if ((offset = lseek(seg->seg_fd, 0, SEEK_CUR)) < 0)
			return -1;

		if ((n = read(seg->seg_fd, &tombstone, sizeof(tombstone))) < 0)
			return -1;

		if (tombstone == 1)
			pair_deleted = 1;

		if (n == 0) // EOF
			break;

		if ((n = read(seg->seg_fd, &val_len, sizeof(val_len))) < 0)
			return -1;

		if (lseek(seg->seg_fd, val_len, SEEK_CUR) < 0)
			return -1;

		if (read(seg->seg_fd, &key_len, sizeof(key_len)) < 0)
			return -1;

		if (read(seg->seg_fd, &key, key_len) < 0)
			return -1;

		if (pair_deleted)
			continue; // skip this pair, its been deleted

		offset += sizeof(tombstone);
		if (segf_update_memtable(seg, key, offset) < 0)
			return -1;

		pair_deleted = 0;
	}

	seg->size = offset;
	return 0;
}

/*
 * Appends the given key value pair to the segment file. This will also
 * add the key and the values offset in the file to the memtable.
 *
 * Params:
 *	seg => pointer to a segment_file that contains the name of the
 *             segment file and the memtable to add to
 *	key => key to add to the file and memtable
 *	val => value to add to the file and memtable
 *	tombstone => byte of metadata associated with key value pair, as of
 *	             now all it does is indicate if the kv pair is being
 *	             deleted. 1 if it is 0 if not.
 *
 * Returns:
 *	-1 if there is an error (check errno), 0 otherwise
 */
int segf_append(struct segment_file *seg, int key, char *val, char tombstone)
{
	unsigned int offset;
	unsigned int kv_pair_sz = 0;
	int val_len, key_len, buf_offset;
	char *buf;

	val_len = strlen(val) + 1; // include null char
	key_len = sizeof(key);
	kv_pair_sz = sizeof(tombstone) + sizeof(val_len) 
			+ val_len + (key_len * 2);

	// allocate space for buffer
	if ((buf = malloc(kv_pair_sz * sizeof(char))) == NULL)
		return -1;
	memset(buf, 0, kv_pair_sz);

	buf_offset = 0;

	// add the tombstone
	memcpy(buf + buf_offset, &tombstone, sizeof(tombstone));
	buf_offset += sizeof(tombstone);

	// add the val length
	memcpy(buf + buf_offset, &val_len, sizeof(val_len));
	buf_offset += sizeof(val_len);

	// add the val
	memcpy(buf + buf_offset, val, val_len);
	buf_offset += val_len;

	// add the key length
	memcpy(buf + buf_offset, &key_len, key_len);
	buf_offset += key_len;

	// add the key
	memcpy(buf + buf_offset, &key, key_len);
	buf_offset += key_len;

	// seek to the end of the file
	if ((offset = lseek(seg->seg_fd, 0, SEEK_END)) < 0) {
		free(buf);
		return -1;
	}

	// handle event where write return n < kv_pair_sz
	if (write(seg->seg_fd, buf, kv_pair_sz) < 0) {
		free(buf);
		return -1;
	}

	offset += sizeof(tombstone); // skip to offset of value length

	// add key and offset to the memtable
	if (segf_update_memtable(seg, key, offset) < 0) {
		free(buf);
		return -1;
	}

	// update the size field
	seg->size += kv_pair_sz;

	free(buf);
	return 0;
}

/*
 * Reads the value using the key from the segment file
 *
 * Params:
 *	seg => pointer a segment_file struct that contains the name of
 *             segment file to read
 *	key => used to look up the value in segment files memtable
 *	val => stores the value read from the segment file
 *
 * Returns:
 *	-1 if there is an error (check errno), 0 otherwise	
 */
int segf_read_file(struct segment_file *seg, int key, char **val)
{
	unsigned int offset;	

	if (memtable_read(seg->table, key, &offset) == 0)
		return -1;
	
	if (lseek(seg->seg_fd, offset, SEEK_SET) < 0)
		return -1;

	int val_len = 0;
	if (read(seg->seg_fd, &val_len, sizeof(val_len)) < 0)
		return -1;

	char *v;
	if ((v = calloc(val_len, sizeof(char))) == NULL)
		return -1;

	if (read(seg->seg_fd, v, val_len) < 0) {
		free(v);
		return -1;
	}
	
	*val = v;
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
