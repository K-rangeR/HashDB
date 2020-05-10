#include <dirent.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/stat.h>
#include <string.h>

#include "hashDB.h"

/* 'Private' helper functions */
static int keep_entry(const struct dirent *);

static char *create_file_path(const char *, const char *);

static char *get_next_segf_name(struct hashDB *);

static struct segment_file *create_segment_file(char*);

static void replace_segf_in_list(struct hashDB*, 
                                 struct segment_file*,
                                 struct segment_file*);

static void add_to_segf_list(struct segment_file**,
                             struct segment_file*,
                             int);

static inline int copy_kv_pair_to(struct segment_file*,
                                  struct segment_file*,
                                  int);

static int merge_possible(struct hashDB*, 
			  struct segment_file**,
			  struct segment_file**);

static int get_size_sum(struct segment_file*, struct segment_file*);

/*
 * Creates a hashDB struct that represents an active database. If data_dir
 * is the name of a directory with segment files in it, a linked list of
 * segment_file structs is created. The list is ordered in descending order 
 * with respect to the segment file names which represent ID's. If data_dir
 * does not exist then it is created and an empty segment file is added to it.
 *
 * Parameter:
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
 * Parameter:
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
	int                  n, i;
	char                 *seg_name;
	struct segment_file  *curr = NULL;

	if ((db = malloc(sizeof(struct hashDB))) == NULL)
		return NULL;

	if ((n = scandir(data_dir, &entries, keep_entry, alphasort)) < 0) {
		free(db);
		return NULL;
	}
	
	db->head = NULL;
	for (i = 0; i < n; ++i) {
		seg_name = create_file_path(data_dir, entries[i]->d_name);
		if (seg_name == NULL)
			break;

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
		if (curr != NULL)
			segf_free(curr);
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
 * Parameter:
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
 * Parameter:
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
 * Parameter:
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
	char                 *file_path = NULL;
	struct segment_file  *first = NULL;
	struct hashDB        *db;

	if (mkdir(data_dir, 0755) < 0)
		return NULL;

	if ((file_path = create_file_path(data_dir, "1.dat")) == NULL)
		goto err;

	if ((first = create_segment_file(file_path)) == NULL)
		goto err;

	if ((db = malloc(sizeof(struct hashDB))) == NULL)
		goto err;
	
	db->next_id = 2;
	db->head = first;
	return db;

err:
	if (first && first->seg_fd != -1)
		segf_delete_file(first);

	if (first) {
		segf_free(first);
		file_path = NULL;
	}

	if (file_path != NULL)
		free(file_path);

	rmdir(data_dir);
	return NULL;
}


/*
 * Deallocates all of the in memory data structures used by the hashDB struct.
 * This includes all segment_file structs and their respective data structures.
 *
 * Parameter:
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
 * Parameters:
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
	unsigned int kv_sz = get_kv_size(key, val_len);

	if ((kv_sz + db->head->size < MAX_SEG_FILE_SIZE)) // normal append
		return segf_append(db->head, key, val, TOMBSTONE_INS);

	if (hashDB_compact(db, db->head) < 0)
		return -1;

	char *name;
	if ((name = get_next_segf_name(db)) == NULL)
		return -1;

	struct segment_file *seg = NULL;
	if ((seg = create_segment_file(name)) == NULL)
		return -1;

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
 * Parameters:
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
 * Parameter:
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
 * Parameters:
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
 * Parameters:
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
 * Compacts the given segment file.
 *
 * Parameters:
 *	db => pointer to the database handler
 *	seg => pointer to the segment file to compact
 *
 * Returns: 
 *	1 if successful, -1 otherwise (check errno)
 */
int hashDB_compact(struct hashDB *db, struct segment_file *seg)
{
	struct segment_file *tmp = NULL;
	char                *tmp_name = NULL;
	char                *seg_tmp_name = NULL;
	char                *old_seg_name = NULL;
	int                  name_changed = 0;

	if ((tmp_name = create_file_path(db->data_dir, "tmp.dat")) == NULL)
		goto err;

	if ((tmp = create_segment_file(tmp_name)) == NULL)
		goto err;

	int key;
	while ((key = segf_next_key(seg)) != -1) {
		if (copy_kv_pair_to(seg, tmp, key) < 0)
			goto err;
	}
	
	old_seg_name = seg->name;
	if ((seg_tmp_name = create_file_path(db->data_dir, "old.dat")) == NULL)
		goto err;

	if (segf_rename_file(seg, seg_tmp_name) < 0)
		goto err;

	name_changed = 1;

	if (segf_rename_file(tmp, old_seg_name) < 0)
		goto err;

	replace_segf_in_list(db, seg, tmp);

	segf_delete_file(seg);
	segf_free(seg);

	struct segment_file *a, *b;
	if (merge_possible(db, &a, &b)) {
		if (hashDB_merge(db, a, b) == -1) {
			return -1;
		}
	}

	return 1;

err:
	// clean up after error
	if (tmp && tmp->seg_fd != -1)
		segf_delete_file(tmp);

	if (tmp) {
		segf_free(tmp);
		tmp_name = NULL;
	}

	if (name_changed)
		segf_rename_file(seg, old_seg_name);

	segf_reset_next_key(seg);

	if (tmp_name != NULL)
		free(tmp_name);

	if (seg_tmp_name != NULL)
		free(seg_tmp_name);

	return -1;
}


/*
 * Creates a new segment file struct and backing segment file
 *
 * Parameters:
 *	name => string representing the name of the new segment file
 *
 * Returns:
 *	a pointer to the new segment_file struct on the heap, NULL if there
 *	is an error
 */
static struct segment_file *create_segment_file(char *name)
{
	struct segment_file *tmp = NULL;
	
	if ((tmp = segf_init(name)) == NULL)
		return NULL;

	if (segf_create_file(tmp) < 0) {
		segf_free(tmp);
		return NULL;
	}

	return tmp;
}


/*
 * Replaces seg in db's linked list of segment files with tmp
 *
 * Parameters:
 *	db => pointer to database resource handler
 *	seg => segment file to replace
 *	tmp => segment file to replace with
 *
 * Returns:
 *	void
 */
static void replace_segf_in_list(struct hashDB *db,
                                 struct segment_file *seg,
                                 struct segment_file *tmp)
{
	if (db->head == seg) {
		tmp->next = db->head->next;		
		db->head = tmp;
		return;
	}

	struct segment_file *curr, *prev;
	curr = prev = db->head;
	while (curr && curr != seg) {
		prev = curr;
		curr = curr->next;
	}
	prev->next = tmp;
	tmp->next = curr->next;
}


/*
 * Creates a string representing a file path in the format dir_name/file_name.
 * 
 * Parameters:
 *	dir_name => directory path
 *	file_name => name of the file to add to the directory path
 *
 * Returns:
 *	A string allocated on the heap, or NULL if there is no memory to
 *	store the string
 */
static char *create_file_path(const char *dir_name, const char *file_name)
{
	int dir_len  = strlen(dir_name);
	int file_len = strlen(file_name);

	// + 2 for '/' and '\0'
	int len = dir_len + file_len + 2;
	char *path = calloc(len, sizeof(char));
	if (path == NULL)
		return NULL;
	
	strncat(path, dir_name, dir_len);
	strncat(path, "/", 1);
	strncat(path, file_name, file_len);

	return path;
}


/*
 * Checks if there are any two pairs of segment files that can be
 * merge into one. For this to be true the sum of the two file sizes must 
 * still be less than the max segment file size.
 *
 * Parameters:
 *	db => pointer to the database handler
 *	a => pointer to the first segment file to be merged
 *	b => pointer to the second segment file to be merged
 *
 * Returns:
 *	1 if there are two segment files that can merged (a and b will
 *	point to them), or 0 otherwise
 */
static int merge_possible(struct hashDB *db, 
			  struct segment_file **a,
			  struct segment_file **b)
{
	struct segment_file *one = db->head;
	struct segment_file *two;
	*a = *b = NULL;

	while (one) {
		two = db->head;
		while (two) {
			if (two == one) {
				two = two->next;
				continue;
			}

			int sum = get_size_sum(one, two);
			if (sum == -1) {
				printf("ERROR: merge_possible\n");
				goto exit;
			} else if (sum < MAX_SEG_FILE_SIZE) {
				*a = one;
				*b = two;
				goto exit;
			}
			two = two->next;
		}
		one = one->next;
	}

exit:
	return (a && b) ? 1 : 0;
}


/*
 * Returns the sum of given segment file sizes
 *
 * Parameters:
 *	a => first segment file
 *	b => second segment file
 *
 * Returns:
 *	
 */
static int get_size_sum(struct segment_file *a, struct segment_file *b)
{
	struct stat file_info;

	// Log errors and return
	if (stat(a->name, &file_info) < 0) {
		printf("ERROR: get_size_sum: %s\n", strerror(errno));
		return -1;
	}
	int a_size = file_info.st_size;

	if (stat(b->name, &file_info) < 0) {
		printf("ERROR: get_size_sum: %s\n", strerror(errno));
		return -1;
	}

	return a_size + file_info.st_size;
}


/*
 * Merges the two given segment files into one. The resulting segment file
 * is given the same name as the newer of the two segment file (the one with
 * larger name ID)
 *
 * Parameters:
 *	db => pointer the database handler
 *	s1 => one of two segment files to merge
 *	s2 => two of two segment files to merge
 *
 * Returns: 
 *	1 if successful, -1 otherwise (check errno)
 */
int hashDB_merge(struct hashDB *db, 
                 struct segment_file *s1, 
                 struct segment_file *s2)
{
	char                *mtemp_name = NULL;
	struct segment_file *mtemp = NULL;
	struct segment_file *newer, *older;

	if ((mtemp_name = create_file_path(db->data_dir, "mtemp.dat")) == NULL)
		goto err;

	if ((mtemp = create_segment_file(mtemp_name)) == NULL)
		goto err;

	int s1_id = get_id_from_fname(s1->name);
	int s2_id = get_id_from_fname(s2->name);
	if (s1_id > s2_id) {
		newer = s1;
		older = s2;
	} else {
		newer = s2;
		older = s1;
	}

	int key;
	while ((key = segf_next_key(newer)) != -1) {
		if (copy_kv_pair_to(newer, mtemp, key) < 0)
			goto err;

		// Remove key value pair from older memtable if present
		memtable_remove(older->table, key);
	}

	while ((key = segf_next_key(older)) != -1) {
		if (copy_kv_pair_to(older, mtemp, key) < 0)
			goto err;
	}
	
	segf_unlink(&(db->head), s1);
	segf_unlink(&(db->head), s2);

	segf_delete_file(s1);
	segf_delete_file(s2);

	segf_rename_file(mtemp, newer->name);

	if (newer == s1)
		add_to_segf_list(&(db->head), mtemp, s1_id);
	else
		add_to_segf_list(&(db->head), mtemp, s2_id);

	segf_free(s1);
	segf_free(s2);

	return 1;
err:
	if (mtemp == NULL)
		free(mtemp_name);

	if (mtemp != NULL) { 
		segf_delete_file(mtemp);	
		segf_free(mtemp);
	}

	segf_reset_next_key(s1);
	segf_reset_next_key(s2);

	return -1;
}


/*
 * Copies the key value pair identified by 'key' from the segment file
 * to the other.
 *
 * Parameters:
 *	from => source segment file of copy	
 *	to => destination segment file of copy
 *	key => key to copy
 *
 * Returns:
 *	0 of copy was successful, -1 otherwise
 */
static inline int copy_kv_pair_to(struct segment_file *from,
                                  struct segment_file *to,
				  int key)
{
	char *val;

	if (segf_read_file(from, key, &val) < 0)
		return -1;

	if (segf_append(to, key, val, TOMBSTONE_INS) < 0) {
		free(val);
		return -1;
	}

	free(val);
	return 0;
}


/*
 * Adds the given segment file to its sorted place by name
 * in the linked list
 *
 * Parameters:
 *	head => double pointer to the start of the linked list
 *	seg => segment file struct to add to the linked list
 *	seg_id => ID of the segment file, this determines where in the
 *	          linked list to place the segment file struct
 *
 * Returns:
 *	void
 */
static void add_to_segf_list(struct segment_file **head,
                             struct segment_file *seg, 
                             int seg_id)
{
	struct segment_file *curr = *head;
	struct segment_file *prev = NULL;

	while (curr) {
		int id = get_id_from_fname(curr->name);
		if (seg_id > id)
			break;
		prev = curr;
		curr = curr->next;
	}

	if (curr == *head) {
		*head = seg;
		(*head)->next = (curr) ? (curr->next) : (NULL);
	} else {
		seg->next = curr;
		prev->next = seg;	
	}
}
