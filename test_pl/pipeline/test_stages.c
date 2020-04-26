#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "test_stages.h"
#include "db.h"


#define ONE_DIGIT_ID_NAME_LEN 5 // X.dat
#define TWO_DIGIT_ID_NAME_LEN 6 // XX.dat
#define TEST_DATA_DIR "test_data"


// Private helper functions
static struct segment_file *create_segf_for_stage(struct stage *s);
static int delete_segf(struct stage *s);
static char *join_file_path(char *path, char *file);
static char *make_segment_file_name(struct stage *s);


int test_segf_get(struct stage *s)
{
	printf("test_segf_get\n");
	return 1;	
}


int test_segf_put(struct stage *s)
{
	printf("test_segf_put\n");
	struct segment_file *tfile = create_segf_for_stage(s);
	printf("%s\n", tfile->name);
	return 1;	
}


int test_segf_delete(struct stage *s)
{
	printf("test_segf_delete\n");
	return 1;	
}


int test_hashdb_get(struct stage *s)
{
	printf("test_hash_get\n");
	return 1;
}


int test_hashdb_put(struct stage *s)
{
	printf("test_hashdb_put\n");
	return 1;
}


int test_hashdb_delete(struct stage *s)
{
	printf("test_hashdb_delete\n");
	return 1;
}


int test_hashdb_repopulate(struct stage *s)
{
	printf("test_hashdb_repopulate\n");
	return 1;
}


int test_hashdb_mkempty(struct stage *s)
{
	printf("test_hashdb_mkempty\n");
	return 1;
}


int test_hashdb_compact(struct stage *s)
{
	printf("test_hashdb_compact\n");
	return 1;
}


int test_hashdb_merge(struct stage *s)
{
	printf("test_hashdb_merge\n");
	return 1;
}


/*
 * Does not perform any tests, just for debugging
 */
int test_nothing(struct stage *s)
{
	/*
	printf("%s | len = %d | ID1 = %d | ID2 = %d\n", s->name, 
		test_data_len(s), id_one(s), id_two(s));
	for (int i = 0; i < test_data_len(s); ++i) {
		printf("%d | %s\n", 
			key_at(s, i),
			value_at(s, i));
	}
	*/
	printf("Nothing to test for '%s', check spelling...\n", s->name);
	return 1;
}


static struct segment_file *create_segf_for_stage(struct stage *s)
{
	char *file_name = make_segment_file_name(s);
	char *path_to_segment_file = join_file_path(TEST_DATA_DIR, file_name);
	free(file_name); // don't need anymore

	struct segment_file *segf = NULL;
	if ((segf = segf_init(path_to_segment_file)) == NULL) {
		printf("ERROR: create_segf_for_stage: no memory\n");
		exit(1);
	}

	// if segment file does not exist, create it
	// else, open it

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
		printf("ERROR: make_segement_file_name: no memory\n");
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
		printf("ERROR: join_file_path: no memory\n");	
		exit(1);
	}

	strncat(new_path, path, path_len);
	strncat(new_path, "/", 1);
	strncat(new_path, file, file_len);
	return new_path;
}
