#ifndef _HASHDB_SEGMENT_FILE_H_
#define _HASHDB_SEGMENT_FILE_H_

#include "memtable.h"

// Represents a segment file that stores the databases key value pairs
struct segment_file {
	unsigned int size;
	char *name;
	int seg_fd;
	struct memtable *table;
	struct segment_file *next;
};

struct segment_file *segf_init(char *name);
void segf_free(struct segment_file *seg);
int segf_create_file(struct segment_file *seg);
int segf_open_file(struct segment_file *seg);
void segf_close_file(struct segment_file *seg);
int segf_delete_file(struct segment_file *seg);
int segf_read_file(struct segment_file *seg, int key, char **val);
int segf_append_tombstone(struct segment_file *seg, char tombstone);
int segf_append(struct segment_file *seg, int key, char *val, char tombstone);
int segf_remove_pair(struct segment_file *seg, int key);
int segf_repop_memtable(struct segment_file *seg);
int segf_update_memtable(struct segment_file *seg, int key, unsigned int offset);
int segf_read_memtable(struct segment_file *seg, int key, unsigned int *offset);
void segf_link_before(struct segment_file *s1, struct segment_file *s2);
void segf_unlink(struct segment_file *head, struct segment_file *seg);

#endif
