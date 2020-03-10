#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>
#include "hashDB.h"

static int keep_entry(const struct dirent *);
static char *get_next_segf_name(struct hashDB *);

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

	if (db)
		db->data_dir = data_dir;
		
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
	struct hashDB        *db;
	struct dirent        **entries; // array of struct dirent pointers
	int                  n, path_len, i;
	char                 *seg_name;
	struct segment_file  *curr = NULL;

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
		printf("ERROR: hashDB.c: hashDB_repopulate: %s\n", 
				strerror(errno));
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
static int keep_entry(const struct dirent *entry)
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
	char                 *test_file_path;
	struct segment_file  *first = NULL;
	struct hashDB        *db;

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

/*
 * Inserts the given key value pair into the database
 *
 * Params:
 *	db => pointer to the database resource handler
 *	key => key to insert
 *	val_len => length of the value (in bytes)
 *	val => pointer to the value to insert
 *
 * Returns:
 *	0 if successful, -1 otherwise (check errno). If there is an error
 *	the database is left unchanged, however a compact and merge may
 *	have taken place but the result of these are invisible to the user.
 */
int hashDB_put(struct hashDB *db, int key, int val_len, char *val)
{
	// get size of the new kv pair
	unsigned int kv_sz = get_kv_size(key, val_len);

	if ((kv_sz + db->head->size < 1014)) // normal append
		return segf_append(db->head, key, val, TOMBSTONE_INS);

	if (hashDB_compact(db, db->head) < 0)
		return -1;

	char *name;
	if ((name = get_next_segf_name(db)) == NULL)
		return -1;

	struct segment_file *seg = NULL;
	if ((seg = segf_init(name)) == NULL)
		return -1;

	if (segf_create_file(seg) < 0) {
		segf_free(seg);
		return -1;
	}

	segf_link_before(seg, db->head);
	db->head = seg;

	// append to the new segment file
	if (segf_append(db->head, key, val, TOMBSTONE_INS) < 0) {
		segf_delete_file(db->head);	
		db->head = db->head->next;
		segf_free(seg);
		return -1;
	}

	db->next_id += 1;
	return 0;
}

/*
 * Calculates and returns the total size in bytes that the key value
 * pair would take up in a segment file.
 *
 * Params:
 *	key => key in the key value pair
 *	val_len => length of the value
 *
 * Returns:
 *	The size of the key length, key, val_len, and value
 */
unsigned int get_kv_size(int key, int val_len)
{
	unsigned int sz = sizeof(char)
			+ (sizeof(key)*2)
			+ sizeof(val_len)
			+ val_len;
	return sz;
}

/*
 * Uses the next_id and data_dir field in the given hashDB struct to
 * build a string representing the file path to a new segment file.
 *
 * File paths be in the following format:
 *	data_dir/[next_id].dat
 *
 * Params:
 *	db => pointer a database handler
 *
 * Returns:
 *	A pointer a dynamically allocated string reprsenting a segment 
 * 	file path, or null if there is no memory available.
 */
static char *get_next_segf_name(struct hashDB *db)
{
	int  path_len, name_len;
	char *path, *name;

	path_len = strlen(db->data_dir) + 1; // +1 for '/'
	name_len = (db->next_id >= 10) ? 7 : 6; // XX.dat or X.dat and '\0'

	path_len += name_len;
	if ((path = calloc(path_len, sizeof(char))) == NULL)
		return NULL;
	
	if ((name = calloc(name_len, sizeof(char))) == NULL) {
		free(path);
		return NULL;
	}

	sprintf(name, "%d.dat", db->next_id);
	strncat(path, "/", 1);
	strncat(path, name, name_len);

	free(name);
	return path;
}

/*
 * Gets the value associated with the given key
 *
 * Params:
 *	db => hashDB to read from
 *	key => used to look up the value
 *	val => pointer to where the value will be stored if the key was found
 *
 * Returns:
 *	-1 if there is an error (check errno), 0 if the key was not found,
 *	of 1 if the key was found
 */
int hashDB_get(struct hashDB *db, int key, char **val)
{
	struct segment_file  *curr = db->head;	
	int                   res;

	while (curr) {
		if ((res = segf_read_file(curr, key, val)) != 0)
			return res;
		curr = curr->next;
	}

	return 0;
}

/*
 * Removes a key value pair from the database
 *
 * Params:
 *	db => pointer to a database handler
 *	key => key to look up and remove
 *
 * Returns:
 *	1 if the key was found and the key value pair was deleted, 0 if the
 *	key was not found, or -1 if there was an error (check errno)
 */
int hashDB_delete(struct hashDB *db, int key)
{
	struct segment_file *curr = db->head;
	int                 res = 0;

	while (curr) {
		// attempt to remove key from current segment file
		if ((res = segf_remove_pair(curr, key)))
			break;
		curr = curr->next;
	}

	return (curr) ? res : 0;
}

/*
 * TODO: Write this function!!!!
 * Returns 1 if successful, -1 otherwise
 */
int hashDB_compact(struct hashDB *db, struct segment_file *seg)
{
	return 0;
}

/*
 * TODO: Write this function!!!!
 * Returns 1 if successful, -1 otherwise
 */
int hashDB_merge(struct hashDB *db, 
                 struct segment_file *s1, 
                 struct segment_file *s2)
{
	return 0;	
}
