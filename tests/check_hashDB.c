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
#include <fcntl.h>

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

	for (int i = 0; i < TOTAL_TEST_FILES; ++i) {
		idx = i * KV_PAIRS_PER_FILE + 1;
		res = hashDB_get(db, tdxl[idx].key, &val);
		if (res == -1 || res == 0)
			ck_abort_msg("ERROR: could not read segment file\n");
		ck_assert_str_eq(val, tdxl[idx].val);
	}

	hashDB_free(db);
} END_TEST


START_TEST(test_hashDB_compact)
{
	const char *compact_file_path = "tdata/compact/2.dat";
	char *fname = calloc(strlen(compact_file_path)+1, sizeof(char));
	if (fname == NULL)
		ck_abort_msg("ERROR: calloc failed\n");
	strcpy(fname, compact_file_path);

	struct segment_file *seg;
	if ((seg = segf_init(fname)) == NULL)
		ck_abort_msg("ERROR: segf_init failed\n");
	if (segf_open_file(seg) < 0)
		ck_abort_msg("ERROR: segf_open_file failed\n");

	// Write testing data to the segment file
	for (int i = 0; i < TOTAL_KV_PAIRS; ++i) {
		if (segf_append(seg, td[i].key, td[i].val, TOMBSTONE_INS) < 0)
			ck_abort_msg("ERROR: segf_append failed\n");
	}

	// Update 2 of the key value pairs in the segment file
	test_kv updated[] = { {2, "twO"}, {4, "fouR"} }; // even
	for (int i = 0; i < 2; ++i) {
		if (segf_append(seg, updated[i].key, 
			        updated[i].val, TOMBSTONE_INS) < 0)
			ck_abort_msg("ERROR: segf_append failed (update)\n");
	}
	
	// Delete 2 others
	int delete[] = {1, 3}; // odd
	for (int i = 0; i < 2; ++i) {
		if (segf_remove_pair(seg, delete[i]) < 0)
			ck_abort_msg("ERROR: segf_remove_pair failed\n");
	}

	segf_free(seg);

	// Populate a new hashDB struct
	const char *data_dir = "tdata/compact";
	struct hashDB *db = hashDB_init(data_dir);
	if (db == NULL)
		ck_abort_msg("ERROR: hashDB_repopulate failed\n");

	// Run compaction
	if (hashDB_compact(db, db->head->next) == -1) // compact 2.dat
		ck_abort_msg("ERROR: hashDB_compact failed\n");

	ck_assert_str_eq(db->head->next->name, compact_file_path);

	// Check that the updated data is there
	for (int i = 0; i < 2; ++i) {
		char *val;
		int res = segf_read_file(db->head->next, updated[i].key, &val);
		if (res != 1) {
			if (res == -1)
				ck_abort_msg("ERROR: segf_read_file failed\n");
			else
				ck_abort_msg("ERROR: key not found\n");
		}
		ck_assert_str_eq(val, updated[i].val);
	}

	// Check that the deleted data is not there
	for (int i = 0; i < 2; ++i) {
		unsigned int offset;
		int a = segf_read_memtable(db->head->next, delete[i], &offset);
		if (a != 0)
			ck_abort_msg("ERROR: deleted key found\n");
	}

	hashDB_free(db);

	// clear the test segment file
	int fd = open(compact_file_path, O_TRUNC);
	close(fd);
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
	tcase_add_test(tc, test_hashDB_compact);

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
