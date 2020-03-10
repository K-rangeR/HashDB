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
 * Note if any of the tests in check_memtable and check_segment fail
 * this test will probably fail to
 */
START_TEST(test_hashDB_repopulate)
{
	extern struct hashDB * hashDB_repopulate(const char *data_dir);
	extern   void hashDB_free(struct hashDB*);

	const char *test_dir = "tdata/hashDB_tdata";

	struct hashDB *db;
	if ((db = hashDB_repopulate(test_dir)) == NULL)
		ck_abort_msg("ERROR: Could not repopulate DB\n");

	ck_assert_int_eq(db->next_id, TOTAL_TEST_FILES + 1);
	
	int i = 0, curr_start = 0, res = 0;
	struct segment_file *seg = db->head;
	while (seg) {
		ck_assert_str_eq(test_file_namesxl[i], seg->name);
		ck_assert_ptr_nonnull(seg->table);

		// check that the key is in the memtable
		unsigned int offset;
		int j = curr_start;
		while (j < KV_PAIRS_PER_FILE) {
			if ((res = segf_read_memtable(seg, 
					tdxl[j].key, &offset)) != 1) {
				if (res == -1)
					ck_abort_msg("ERROR: memtable error\n");
				
				// first pair is deleted
				if (res == 0 && j != curr_start)
					ck_abort_msg("ERROR: key not found\n");
			}
			++j;
		}

		// check that the offset is correct
		j = curr_start;
		while (j < KV_PAIRS_PER_FILE) {
			char *val;
			if ((res = segf_read_file(seg, tdxl[j].key, 
					&val)) <= 0) {
				if (res == -1)
					ck_abort_msg("ERROR: read fail\n");
				if (res == 0 && j != curr_start)
					ck_abort_msg("ERROR: key not found\n");
			}

			ck_assert_str_eq(val, tdxl[j].val);
			free(val);
			++j;
		}

		curr_start = j;
		seg = seg->next;
		++i;
	}

	hashDB_free(db);
} END_TEST

START_TEST(test_hashDB_mkempty)
{
	extern struct hashDB * hashDB_mkempty(const char *);	
	extern   void hashDB_free(struct hashDB*);

	const char *test_dir = "tdata/hashDB_tdata/mkempty";
	struct hashDB *tdb;
	if ((tdb = hashDB_mkempty(test_dir)) == NULL)
		ck_abort_msg("ERROR: could not create hashDB: %s\n",
				strerror(errno));

	// check the directory was created
	DIR *tdir = opendir(test_dir);
	if (!tdir || errno == ENOENT)
		ck_abort_msg("ERROR: directory not created\n");
	closedir(tdir);

	// check the first segment file was created
	int err = access("tdata/hashDB_tdata/mkempty/1.dat", F_OK);
	if (err == -1 && errno == ENOENT) {
		ck_abort_msg("ERROR: first segment file does not exist: %s\n",
			strerror(errno));
	} else if (err == -1 && errno != 0) {
		ck_abort_msg("ERROR: %s\n", strerror(errno));
	}

	// check that the segment file struct was created
	ck_assert_ptr_nonnull(tdb->head);
	ck_assert_str_eq(tdb->head->name, "tdata/hashDB_tdata/mkempty/1.dat");

	// check that the next ID is correct
	ck_assert_int_eq(tdb->next_id, 2);

	hashDB_free(tdb);
} END_TEST

START_TEST(test_hashDB_get)
{
	extern struct hashDB * hashDB_init(const char *);	
	extern int hashDB_get(struct hashDB *, int, char**);
	extern void hashDB_free(struct hashDB *);

	int res, idx;
	char *val;
	struct hashDB *db;

	const char *data_dir = "tdata/hashDB_tdata";
	if ((db = hashDB_init(data_dir)) == NULL)
		ck_abort_msg("ERROR: create hashDB: %s\n", strerror(errno));

	ck_assert_str_eq(db->data_dir, data_dir);

	// try and get from each segment file
	for (int i = 0; i < TOTAL_TEST_FILES; ++i) {
		idx = i * KV_PAIRS_PER_FILE + 1;
		res = hashDB_get(db, tdxl[idx].key, &val);
		if (res == -1 || res == 0)
			ck_abort_msg("ERROR: could not read segment file\n");
		ck_assert_str_eq(val, tdxl[idx].val);
	}

	hashDB_free(db);
} END_TEST

Suite *hashDB_suite(void)
{
	Suite *s;
	TCase *tc;

	s = suite_create("hashDB Repopulate");
	tc = tcase_create("Core");

	tcase_add_test(tc, test_hashDB_repopulate);
	tcase_add_test(tc, test_hashDB_get);
	tcase_add_test(tc, test_hashDB_mkempty);

	suite_add_tcase(s, tc);
	return s;
}

START_TEST(test_get_id_from_fname)
{
	extern int get_id_from_fname(const char *);	

	int res;

	char *tn1 = "testdir/1.dat";
	res = get_id_from_fname(tn1);
	ck_assert_int_eq(res, 1);

	char *tn2 = "testdir/subdir/2.dat";
	res = get_id_from_fname(tn2);
	ck_assert_int_eq(res, 2);

	char *tn3 = "testdir/11.dat";
	res = get_id_from_fname(tn3);
	ck_assert_int_eq(res, 11);

	char *tn4 = "testdir/1";
	res = get_id_from_fname(tn4);
	ck_assert_int_eq(res, -1);
} END_TEST

Suite *util_suite(void)
{
	Suite *s;
	TCase *tc;

	s = suite_create("Utils");
	tc = tcase_create("Core");

	tcase_add_test(tc, test_get_id_from_fname);

	suite_add_tcase(s, tc);
	return s;
}

int main(void)
{
	int fail = 0;
	Suite *s, *s2;
	SRunner *runner;

	s = hashDB_suite();
	s2 = util_suite();
	runner = srunner_create(s);
	srunner_add_suite(runner, s2);

	srunner_run_all(runner, CK_NORMAL);
	fail = srunner_ntests_failed(runner);
	srunner_free(runner);

	remove("./tdata/hashDB_tdata/mkempty/1.dat");
	rmdir("./tdata/hashDB_tdata/mkempty");

	return (fail == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
