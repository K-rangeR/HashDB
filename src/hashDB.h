#ifndef _HASHDB_HASHDB_H_
#define _HASHDB_HASHDB_H_

#include "segment.h"

// Represents a database handler. Through this users can interact with
// the data stored in the semgent files
struct hashDB {
	// start of the linked list of active segment files
	struct segment_file *head;

	// ID to be given to the next newly created segment file
	int next_id;
};

struct hashDB *hashDB_init(const char *data_dir);
struct hashDB *hashDB_repopulate(const char *data_dir);
struct hashDB *hashDB_mkempty(const char *data_dir);
void hashDB_free(struct hashDB *db);
int hashDB_put(struct hashDB *db, int key, int val_len, char *val);
int hashDB_get(struct hashDB *db, int key, char **val);
int hashDB_delete(struct hashDB *db, int key);
int hashDB_compact(struct hashDB *db,
                   struct segment_file *seg);
int hashDB_merge(struct hashDB *db,
                 struct segment_file *s1,
                 struct segment_file *s2);
int get_id_from_fname(const char *);
unsigned int get_kv_size(int key, int val_len);

#endif
