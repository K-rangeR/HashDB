#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>
#include "hashDB.h"

int keep_entry(const struct dirent *);

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
		db = hashDB_repopulate(data_dir);	
	} else if (errno == ENOENT) { // does not exist
		dir = NULL;
		db = hashDB_mkempty(data_dir);	
	} 

	return db;
}

struct hashDB *hashDB_repopulate(const char *data_dir)
{
	struct hashDB *db;
	struct dirent **entries; // array of struct dirent pointers
	int n, path_len;
	char *seg_name;
	struct segment_file *curr = NULL;

	if ((db = malloc(sizeof(struct hashDB))) == NULL)
		return NULL;

	if ((n = scandir(data_dir, &entries, keep_entry, alphasort)) < 0)
		return NULL;
	
	db->head = NULL;
	for (int i = 0; i < n; ++i) {
		// + 2 for '/' and the null terminator
		path_len = strlen(data_dir) + strlen(entries[i]->d_name) + 2;
		seg_name = calloc(path_len, sizeof(char));

		// build file path
		strcat(seg_name, data_dir);
		strcat(seg_name, "/");
		strcat(seg_name, entries[i]->d_name);

		if ((curr = segf_init(seg_name)) == NULL) {
			hashDB_free(db);
			free(entries[i]);
			break; // MEMORY LEAK!!! remaining entries are not free
		}

		if (segf_open_file(curr) < 0)
			printf("segf_open_file failed: %s\n", strerror(errno));
		
		// read the current segment file

		segf_link_before(curr, db->head);
		db->head = curr;

		free(entries[i]);
	}

	free(entries);
	return db;
}

int keep_entry(const struct dirent *entry)
{
	if ((strcmp(entry->d_name, ".") == 0) ||
	    (strcmp(entry->d_name, "..") == 0)) {
		return 0;
	}
	return 1;
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

void hashDB_free(struct hashDB *db)
{
	struct segment_file *curr, *prev;	

	curr = prev = db->head;
	while (curr) {
		curr = curr->next;
		segf_free(prev);
		prev = curr;
	}

	free(db);
	db = NULL;
}
