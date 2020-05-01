#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include "test_stages.h"
#include "db.h"


#define ONE_DIGIT_ID_NAME_LEN 5 // X.dat
#define TWO_DIGIT_ID_NAME_LEN 6 // XX.dat
#define TEST_DATA_DIR "test_data"


// Private helper functions
static int test_segf_get(struct stage *, struct segment_file *);
static struct segment_file *create_segf_for_stage(struct stage *s);
static char *join_file_path(char *path, char *file);
static char *make_segment_file_name(struct stage *s);


/*
 * Attempts to read the data associated with the given stage file
 */
static int test_segf_get(struct stage *s, struct segment_file *tfile)
{
	for (int i = 0; i < test_data_len(s); ++i) {
		char *val;
		if (segf_read_file(tfile, key_at(s,i), &val) <= 0) {
			printf("[F]: test_segf_get: read: %s\n", 
				strerror(errno));
			return 0;
		}

		if (strcmp(val, value_at(s,i)) != 0) {
			printf("[F]: test_segf_get: '%s' != '%s'\n", val,
				value_at(s, i));
			return 0;
		}
		printf("'%s' == '%s'\n", val, value_at(s, i));
		free(val);
	}
	return 1;	
}


/*
 * Stage for testing segment file insert code
 */
int test_segf_put(struct stage *s)
{
	struct segment_file *tfile = create_segf_for_stage(s);
	if (!tfile)
		return 0;

	for (int i = 0; i < test_data_len(s); ++i) {
		if (segf_append(tfile, key_at(s,i), 
				value_at(s,i), TOMBSTONE_INS) < 0) {
			printf("[F]: test_segf_put: append: %s\n", 
				strerror(errno));
			return 0;
		}
	}

	int result = test_segf_get(s, tfile);
	segf_free(tfile);
	return result;
}


/*
 * Stage for testing segment file delete code
 */
int test_segf_delete(struct stage *s)
{
	struct segment_file *tfile = create_segf_for_stage(s);
	if (!tfile)
		return 0;

	for (int i = 0; i < test_data_len(s); ++i) {
		int res = segf_remove_pair(tfile, key_at(s, i));
		if (res == 0) {
			printf("[F]: test_segf_delete: could not find '%d'\n",
				key_at(s, i));
			return 0;
		} else if (res == -1) {
			printf("[!]: test_segf_delete: %s\n", strerror(errno));
			return 0;
		}
	}

	for (int i = 0; i < test_data_len(s); ++i) {
		char *val;
		int res = segf_read_file(tfile, key_at(s,i), &val);
		if (res == 1) {
			printf("[F]: test_segf_delete: '%d' was not deleted\n",
				key_at(s,i));
			return 0;
		} else if (res == -1) {
			printf("[!]: test_segf_delete: %s\n", strerror(errno));	
			return 0;
		}
	}

	segf_free(tfile);
	return 1;
}


/*
 * Tests the hashDB_put functions
 */
int test_hashdb_put(struct stage *s)
{
	struct hashDB *tdb = NULL;
	if ((tdb = hashDB_init(TEST_DATA_DIR)) == NULL) {
		printf("[F]: test_hashdb_put: hashDB_init: %s\n", 
			strerror(errno));
		return 0;
	}

	for (int i = 0; i < test_data_len(s); ++i) {
		int val_len = strlen(value_at(s,i));	
		if (hashDB_put(tdb, key_at(s,i), val_len, value_at(s,i)) < 0) {
			printf("[F]: test_hashdb_put: hashDB_put: %s\n", 
				strerror(errno));
			hashDB_free(tdb);
			return 0;
		}
	}

	for (int i = 0; i < test_data_len(s); ++i) {
		char *val;
		int res = hashDB_get(tdb, key_at(s,i), &val);
		if (res == -1) {
			printf("[!]: test_hashdb_put: hashDB_get: %s\n", 
				strerror(errno));
			free(val);
			hashDB_free(tdb);
			return 0;
		} else if (res == 0) {
			printf("[F]: test_hashdb_put: hashDB_get: %d not found\n", 
				key_at(s,i));
			hashDB_free(tdb);
			return 0;
		}
		printf("'%s' == '%s'\n", value_at(s,i), val);
		free(val);
	}

	hashDB_free(tdb);
	return 1;
}


/*
 * Tests hashDB_delete function
 */
int test_hashdb_delete(struct stage *s)
{
	struct hashDB *tdb = NULL;
	if ((tdb = hashDB_init(TEST_DATA_DIR)) == NULL) {
		printf("[F]: test_hashdb_delete : hashDB_init: %s\n", 
			strerror(errno));
		return 0;
	}

	for (int i = 0; i < test_data_len(s); ++i) {
		if (hashDB_delete(tdb, key_at(s,i)) < 0) {
			printf("[F]: test_hashdb_delete: hashDB_delete: %s\n", 
				strerror(errno));
			hashDB_free(tdb);
			return 0;
		}
	}

	for (int i = 0; i < test_data_len(s); ++i) {
		char *val;
		int res = hashDB_get(tdb, key_at(s,i), &val);
		if (res == -1) {
			printf("[!]: test_hashdb_delete: hashDB_get: %s\n", 
				strerror(errno));
			free(val);
			hashDB_free(tdb);
			return 0;
		} else if (res == 1) {
			printf("[F]: test_hashdb_delete: hashDB_get: %d found\n", 
				key_at(s,i));
			free(val);
			hashDB_free(tdb);
			return 0;
		}
	}

	hashDB_free(tdb);
	return 1;
}


/*
 * Tests hashDB_compact function
 */
int test_hashdb_compact(struct stage *s)
{
	struct hashDB *tdb = NULL;
	if ((tdb = hashDB_init(TEST_DATA_DIR)) == NULL) {
		printf("[F]: test_hashdb_compact: hashDB_init: %s\n", 
			strerror(errno));
		return 0;
	}

	struct segment_file *segf = NULL;
	struct segment_file *curr = tdb->head;
	while (curr) {
		int curr_id = get_id_from_fname(curr->name);
		if (curr_id == id_one(s)) {
			segf = curr;	
			break;
		}
		curr = curr->next;
	}

	if (segf == NULL) {
		printf("[!]: test_hashdb_compact: could not find ID\n");
		hashDB_free(tdb);	
		return 0;
	}

	if (hashDB_compact(tdb, segf) == -1) {
		printf("[F]: test_hashdb_compact: hashDB_compact: %s\n",
			strerror(errno));
		hashDB_free(tdb);
		return 0;
	}

	hashDB_free(tdb);
	return 1;
}


/*
 * Tests the hashDB_merge function
 */
int test_hashdb_merge(struct stage *s)
{
	struct hashDB *tdb = NULL;
	if ((tdb = hashDB_init(TEST_DATA_DIR)) == NULL) {
		printf("[F]: test_hashdb_compact: hashDB_init: %s\n", 
			strerror(errno));
		return 0;
	}

	struct segment_file *one = NULL, *two = NULL;
	struct segment_file *curr = tdb->head;
	while (curr) {
		int curr_id = get_id_from_fname(curr->name);
		if (curr_id == id_one(s)) {
			one = curr;
		}
		if (curr_id == id_two(s)) {
			two = curr;
		}
		curr = curr->next;
	}

	if (one == NULL || two == NULL) {
		printf("[!]: test_hashdb_compact: could not find ID\n");
		hashDB_free(tdb);	
		return 0;
	}

	if (hashDB_merge(tdb, one, two) == -1) {
		printf("[F]: test_hashdb_merge: hashDB_merge: %s\n", 
			strerror(errno));
		hashDB_free(tdb);
		return 0;
	}

	hashDB_free(tdb);
	return 1;
}


/*
 * Does not perform any tests, just for debugging
 */
int test_nothing(struct stage *s)
{
	printf("Nothing to test for '%s', check spelling...\n", s->name);
	return 1;
}


/*
 * Creates new segment file struct for testing a specific segf function.
 * If the backing segment file does not exist one is created. If there
 * is an error NULL is returned.
 */
static struct segment_file *create_segf_for_stage(struct stage *s)
{
	char *file_name = make_segment_file_name(s);
	char *path_to_segment_file = join_file_path(TEST_DATA_DIR, file_name);
	free(file_name); // don't need anymore

	struct segment_file *segf = NULL;
	if ((segf = segf_init(path_to_segment_file)) == NULL) {
		printf("[!]: create_segf_for_stage: no memory\n");
		return NULL;
	}

	if (access(path_to_segment_file, F_OK) != -1) {
		if (segf_open_file(segf) < 0) {
			printf("[F]: Could not open file\n");
			printf("\t-> %s\n", strerror(errno));
			return NULL;
		}
		if (segf_repop_memtable(segf) < 0) {
			printf("[F]: Could not repopulate memtable\n");
			printf("\t-> %s\n", strerror(errno));
			return NULL;
		}
	} else {
		if (segf_create_file(segf) < 0) {
			printf("[F]: Could create segment file\n");
			printf("\t-> %s\n", strerror(errno));
			return NULL;	
		}
	}

	return segf;
}


/*
 * Uses the ID from the stage struct to create a segment file name. If there
 * is no memory for the file name an error message is printed and the program
 * is terminated.
 */
static char *make_segment_file_name(struct stage *s)
{
	int name_len = (id_one(s) < 10) ? ONE_DIGIT_ID_NAME_LEN 
					: TWO_DIGIT_ID_NAME_LEN;

	char *name = calloc(name_len+1, sizeof(char));
	if (!name) {
		printf("[!]: make_segement_file_name: no memory\n");
		exit(1);
	}

	sprintf(name, "%d.dat", id_one(s));
	return name;
}


/*
 * Creates a new string with path and file separated by a '/'
 */
static char *join_file_path(char *path, char *file)
{
	int path_len = strlen(path), file_len = strlen(file);
	int len = path_len + file_len + 2;
	char *new_path = calloc(len, sizeof(char));
	if (!new_path) {
		printf("[!]: join_file_path: no memory\n");	
		exit(1);
	}

	strncat(new_path, path, path_len);
	strncat(new_path, "/", 1);
	strncat(new_path, file, file_len);
	return new_path;
}
