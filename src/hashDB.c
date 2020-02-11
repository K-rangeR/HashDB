#include <dirent.h>
#include <errno.h>
#include "hashDB.h"

/*
 * Creates a hashDB struct that represents an active database. If data_dir
 * is the name of a directory with segment files in it, a linked list of
 * segment_file structs is created. The list is ordered in descending order 
 * with respect to the segment file names which represent ID's. If data_dir
 * does not exist then it is created and an empty segment file is added to it.
 *
 * Params:
 *	data_dir => name of a directory to read from or create
 *
 * Returns:
 *	Dynamically allocated hashDB struct that is ready to handle read and
 *	write requests. This struct must be deallocated using hashDB_free. If
 *	there is an error NULL is returned.
 */
struct hashDB *hashDB_init(const char *data_dir)
{
	struct hashDB *db = NULL;
	DIR *dir = opendir(data_dir);
	if (dir) {
		closedir(dir);
		db = hashDB_repopulate();	
	} else if (errno == ENOENT) { // does not exist
		dir = NULL;
		db = hashDB_mkempty(data_dir);	
	} 

	return db;
}

struct hashDB *hashDB_repopulate()
{
	return NULL;
}

struct hashDB *hashDB_mkempty(const char *data_dir)
{
	if (mkdir(data_dir, 0755) < 0)
		return NULL;

	struct segment_file *first;
	if ((first = segf_init("1.dat")) == NULL)
		return NULL;

	if (segf_create_file(first) < 0) {
		segf_free(first);
		return NULL;
	}

	struct hashDB *db;
	if ((db = malloc(sizeof(struct hashDB))) == NULL) {
		segf_delete_file(first);
		segf_free(first);
		return NULL;
	}
	
	db->next_id = 2;
	db->head = first;
	return db;
}
