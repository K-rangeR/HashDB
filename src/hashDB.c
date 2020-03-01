#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
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

/*
 * Reads from the given data directory path and builds an in memory list
 * of segment_file structs that represent each of the segment files in the
 * data directory. The segment_file struct memtables are repopulated with
 * the most recent key value pairs found in the segment file.
 *
 * Params:
 *	data_dir => name of a directory containing segment files
 *
 * Returns:
 *	Dynamically allocated hashDB struct that represents the key value
 *	database. If there is any error repopulating the database NULL is
 *	returned and errno is set. 

 *	Note, the returned hashDB struct must be deallocated with hashDB_free
 *	when it is no longer needed.
 */
struct hashDB *hashDB_repopulate(const char *data_dir)
{
	struct hashDB *db;
	struct dirent **entries; // array of struct dirent pointers
	int n, path_len, i;
	char *seg_name;
	struct segment_file *curr = NULL;

	if ((db = malloc(sizeof(struct hashDB))) == NULL)
		return NULL;

	if ((n = scandir(data_dir, &entries, keep_entry, alphasort)) < 0)
		return NULL;
	
	db->head = NULL;
	for (i = 0; i < n; ++i) {
		// + 2 for '/' and '\0'
		path_len = strlen(data_dir) + strlen(entries[i]->d_name) + 2;
		seg_name = calloc(path_len, sizeof(char));

		// build file path
		strcat(seg_name, data_dir);
		strcat(seg_name, "/");
		strcat(seg_name, entries[i]->d_name);

		if ((curr = segf_init(seg_name)) == NULL)
			break;

		if (segf_open_file(curr) < 0)
			break;

		if (segf_repop_memtable(curr) < 0)
			break;

		segf_link_before(curr, db->head);
		db->head = curr;

		if (i == n-1)
			db->next_id = get_id_from_fname(entries[i]->d_name)+1;

		free(entries[i]);
	}
	
	if (i < n) { // clean up after error
		printf("ERROR: hashDB_repopulate: %s\n", strerror(errno));
		hashDB_free(db);	
		db = NULL;
		if (curr != NULL) {
			if (curr->seg_fd != -1)
				segf_close_file(curr);
			segf_free(curr);
		}
		while (i < n) {
			free(entries[i]);
			i++;
		}
	}

	free(entries);
	return db;
}

/*
 * Predicate function used by scandir to determine if a directory entry
 * should be included in the array of sorted directory entries. It returns
 * true if the file in data directory is an appropiately named segment file
 * and false otherwise.
 *
 * Param:
 *	entry => pointer a dirent struct that is having its name checked
 *
 * Returns:
 *	1 if dirent->d_name is a valid segment file name and 0 otherwise
 */
int keep_entry(const struct dirent *entry)
{
	if ((strcmp(entry->d_name, ".") == 0) ||
	    (strcmp(entry->d_name, "..") == 0)) {
		return 0;
	}
	return 1;
}

/*
 * Gets the segment file ID from the segment files path
 *
 * Param:
 *	path => path to the segment file
 *
 * Returns:
 *	segment files ID if the name was in the correct format
 *	-1 otherwise
 */
int get_id_from_fname(const char *path)
{
	int i = strlen(path);
	int j = 0;
	while (i >= 0) {
		if (path[i] == '.') // start of the file extention
			j = i;
		if (path[i] == '/') // last '/' in file path
			break;
		--i;
	}

	if (j <= i) {
		printf("%s is not in the correct format\n", path);
		return -1;
	}

	// File ID will be between i and j
	char id[5];
	int k = 0;
	i += 1; // skip '/'
	while (i < j) {
		id[k] = path[i];
		++i;
		++k;
	}

	return atoi(id);
}

/*
 * Creates a directory named after the given string. The first empty
 * segment file is also created in the directory.
 *
 * Param:
 *	data_dir => name of the data directory to create
 *
 * Returns:
 *	Pointer a dynamically allocated hashDB struct that represents the
 *	new key value database.
 *
 *	Note, the returned hashDB struct must be deallocated with hashDB_free
 *	when it is no longer needed.
 */
struct hashDB *hashDB_mkempty(const char *data_dir)
{
	char *test_file_path;
	struct segment_file *first = NULL;
	struct hashDB *db;

	if (mkdir(data_dir, 0755) < 0)
		return NULL;
	
	// +7 = '/' + 1.dat\0
	test_file_path = calloc(strlen(data_dir)+7, sizeof(char));
	if (test_file_path == NULL)
		goto err;

	// build the path to the first test file
	strcat(test_file_path, data_dir);
	strcat(test_file_path, "/");
	strcat(test_file_path, "1.dat");

	if ((first = segf_init(test_file_path)) == NULL)
		goto err;

	if (segf_create_file(first) < 0)
		goto err;

	if ((db = malloc(sizeof(struct hashDB))) == NULL)
		goto err;
	
	db->next_id = 2;
	db->head = first;
	return db;

err:
	if (first && first->seg_fd != -1)
		segf_delete_file(first);
	if (first)
		segf_free(first);
	rmdir(data_dir);
	return NULL;
}

/*
 * Deallocates all of the in memory data structures used by the hashDB struct.
 * This includes all segment_file structs and their respective data structures.
 *
 * Param:
 *	db => pointer to the hashDB struct to free
 *
 * Returns:
 *	void
 */
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
