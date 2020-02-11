#ifndef _HASHDB_HASHDB_H_
#define _HASHDB_HASHDB_H_

#include "segment.h"

// Represents the database
struct hashDB {
	// start of the linked list of active segment files
	struct segment_file *head;
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
                   struct segment_file *s1,
                   struct segment_file *s2);
int hashDB_merge(struct hashDB *db,
                 struct segment_file *s1,
                 struct segment_file *s2);

#endif
