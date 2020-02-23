/*
 * Tests for hashDB.c (these are hardly unit tests anymore)
 */
#include <check.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>
#include "../src/hashDB.h"
#include "data.h"

/*
 * Adds the testing data to the testing segment files. This happens each
 * time the test suite is run. The testing segment files are assumed to be
 * created.
 */
void add_testing_data()
{
	struct segment_file *seg;

	for (int i = 0; i < TOTAL_TEST_FILES; ++i) { // test files
		if ((seg = segf_init(test_file_names[i])) == NULL) {
			printf("ERROR: could not create segment struct: ");
			printf("%s\n", strerror(errno));
			exit(1);
		}

		if (segf_open_file(seg) < 0) {
			printf("ERROR: could not create segment struct: ");
			printf("%s\n", strerror(errno));
			exit(1);
		}

		for (int j = 0; j < TOTAL_KV_PAIRS; ++j) {
			if (segf_append(seg, td[j].key, td[j].val, 0) < 0) {
				printf("ERROR: append failed: ");
				printf("%s\n", strerror(errno));
				exit(1);
			}
		}

		memtable_free(seg->table);
		free(seg);
		seg = NULL;
	}
}

/*
 * Deletes all the testing data from the testing segment files. This does
 * not delete the testing files.
 */
void delete_testing_data()
{
	for (int i = 0; i < TOTAL_TEST_FILES; ++i) {
		if (truncate(test_file_names[i], 0) < 0) {
			printf("ERROR: could not truncate file: ");
			printf("%s\n", strerror(errno));
		}
	}
}

/*
 * Note if any of the tests in check_memtable and check_segment fail
 * this test will probably fail to
 */
START_TEST(test_hashDB_repopulate)
{
	extern struct hashDB * hashDB_repopulate(const char *data_dir);
	extern void hashDB_free(struct hashDB*);

	const char *test_dir = "tdata";

	struct hashDB *db;
	if ((db = hashDB_repopulate(test_dir)) == NULL)
		ck_abort_msg("ERROR: Could not repopulate DB\n");
	
	int i = 0;
	int j;
	char *val;
	unsigned int offset;
	struct segment_file *seg = db->head;
	while (seg) {
		ck_assert_str_eq(test_file_names[i], seg->name);
		ck_assert_ptr_nonnull(seg->table);

		// check that the key is in the memtable
		j = 0;
		while (j < TOTAL_KV_PAIRS) {
			if (!segf_read_memtable(seg, td[j].key, &offset))
				ck_abort_msg("ERROR: key not found\n");
			++j;
		}

		ck_assert_int_eq(seg->size, FILE_SIZE);

		// check that the offset is correct
		j = 0;
		while (j < TOTAL_KV_PAIRS) {
			if (segf_read_file(seg, td[j].key, &val) <= 0)
				ck_abort_msg("ERROR: could not read file\n");
			ck_assert_str_eq(val, td[j].val);
			free(val);
			++j;
		}

		seg = seg->next;
		++i;
	}

	hashDB_free(db);
} END_TEST

START_TEST(test_hashDB_mkempty)
{
	extern struct hashDB * hashDB_mkempty(const char *);	
	extern void hashDB_free(struct hashDB*);

	struct hashDB *tdb;
	if ((tdb = hashDB_mkempty("2_tdata")) == NULL)
		ck_abort_msg("ERROR: could not create hashDB\n");

	// check the directory was created
	DIR *tdir = opendir("2_tdata");
	if (!tdir || errno == ENOENT)
		ck_abort_msg("ERROR: directory not created\n");
	closedir(tdir);

	// check the first segment file was created
	int err = access("2_tdata/1.dat", F_OK);
	if (err == -1 && errno == ENOENT) {
		ck_abort_msg("ERROR: first segment file does not exist: %s\n",
			strerror(errno));
	} else if (err == -1 && errno != 0) {
		ck_abort_msg("ERROR: %s\n", strerror(errno));
	}

	// check that the segment file struct was created
	ck_assert_ptr_nonnull(tdb->head);
	ck_assert_str_eq(tdb->head->name, "2_tdata/1.dat");

	// check that the next ID is correct
	ck_assert_int_eq(tdb->next_id, 2);

	hashDB_free(tdb);
} END_TEST

Suite *hashDB_suite(void)
{
	Suite *s;
	TCase *tc;

	s = suite_create("hashDB Repopulate");
	tc = tcase_create("Core");

	tcase_add_test(tc, test_hashDB_repopulate);
	tcase_add_test(tc, test_hashDB_mkempty);

	suite_add_tcase(s, tc);
	return s;
}

int main(void)
{
	int fail = 0;
	Suite *s;
	SRunner *runner;

	add_testing_data();

	s = hashDB_suite();
	runner = srunner_create(s);

	srunner_run_all(runner, CK_NORMAL);
	fail = srunner_ntests_failed(runner);
	srunner_free(runner);

	delete_testing_data();	

	return (fail == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
