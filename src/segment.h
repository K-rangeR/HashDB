#ifndef _HASHDB_SEGMENT_FILE_H_
#define _HASHDB_SEGMENT_FILE_H_

#include "memtable.h"

// Use a small max segment file size when running tests
#ifdef TESTING
#define MAX_SEG_FILE_SIZE 100
#else
#define MAX_SEG_FILE_SIZE 1024
#endif

// Represents a segment file that stores the databases key value pairs
struct segment_file {
	// size in bytes of the segment file
	unsigned int size;

	// name of the segment file (allocated on heap)
	char *name;

	// file descriptor of open segment file
	int seg_fd;

	// index of the next hash bucket used by segf_next_key
	int next_bucket;

	// next entry in the bucket chain used by segf_next_key
	struct memtable_entry *next_entry;

	// pointer to segment files memtable
	struct memtable *table;

	// pointer to the next (older) segment file struct
	struct segment_file *next;
};

#define TOMBSTONE_INS 0 // tombstone for inserting a kv pair
#define TOMBSTONE_DEL 1 // tombstone for deleting a kv pair

/* Struct constructors and destructors */
struct segment_file *segf_init(char *name);
void segf_free(struct segment_file *seg);

/* Segment file IO functions */
int segf_create_file(struct segment_file *seg);
int segf_open_file(struct segment_file *seg);
void segf_close_file(struct segment_file *seg);
int segf_delete_file(struct segment_file *seg);
int segf_read_file(struct segment_file *seg, int key, char **val);
int segf_append(struct segment_file *seg, int key, char *val, char tombstone);
int segf_remove_pair(struct segment_file *seg, int key);

/* Segment file memtable functions */
int segf_repop_memtable(struct segment_file *seg);
int segf_update_memtable(struct segment_file *seg, int key, unsigned int offset);
int segf_read_memtable(struct segment_file *seg, int key, unsigned int *offset);

/* Segment file linked list functions */
void segf_link_before(struct segment_file *s1, struct segment_file *s2);
void segf_unlink(struct segment_file *head, struct segment_file *seg);

#endif
