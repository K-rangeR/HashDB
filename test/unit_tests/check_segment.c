/* 
 * Tests for segment.c
 */
#include <check.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>

#include "../../src/segment.h"

#define TEST_FILE_PATH "tdata/segment_tdata/1.dat"
#define TEST_FILE_PATH_2 "tdata/segment_tdata/2.dat"
#define TEST_FILE_PATH_LEN 25

START_TEST(test_segf_init)
{
	extern struct segment_file *segf_init(char*);
	extern   void segf_free(struct segment_file*);

	char *tname = malloc(sizeof(char) * 9);
	if (tname == NULL)
		ck_abort_msg("Could not malloc space for file name\n");
	strcpy(tname, "test.dat");

	struct segment_file *seg;
	if ((seg = segf_init(tname)) == NULL)
		ck_abort_msg("Could not create segment file struct\n");
	
	ck_assert_int_eq(seg->size, 0);
	ck_assert_str_eq(seg->name, tname);
	ck_assert_int_eq(seg->seg_fd, -1);
	ck_assert_int_eq(seg->next_bucket, 0);
	ck_assert_ptr_null(seg->next_entry);
	ck_assert_ptr_nonnull(seg->table);
	ck_assert_ptr_null(seg->next);

	segf_free(seg);
} END_TEST


START_TEST(test_segf_link_before)
{
	extern void segf_link_before(struct segment_file*, struct segment_file*);

	struct segment_file s1, s2;

	s1.name = "s1.dat";
	s1.next = NULL;

	s2.name = "s2.dat";
	s2.next = NULL;

	segf_link_before(&s1, &s2);
	ck_assert_ptr_eq(s1.next, &s2);
	ck_assert_ptr_null(s2.next);
} END_TEST


START_TEST(test_segf_next_key)
{
	extern struct segment_file *segf_init(char*);
	extern int segf_next_key(struct segment_file*);
	extern void segf_free(struct segment_file*);

	struct segment_file *seg;
	if ((seg = segf_init("test")) == NULL)
		ck_abort_msg("ERROR: segf_init failed\n");

	struct key_found_pair {
		int key;     // testing key
		char found;  // flag = 1 if key was found, flag = 0 if not
	};

	struct key_found_pair test_data[] = {
		{0, 0},	
		{1, 0},	
		{2, 0},	
		{3, 0},	
		{4, 0},	
	};

	// Add some testing key offset data
	for (int i = 0; i < 5; ++i) {
		int key = test_data[i].key;
		if (segf_update_memtable(seg, key, key+1) < 0)
			ck_abort_msg("ERROR: segf_update_memtable failed\n");
	}

	int key, cnt = 5;
	while ((key = segf_next_key(seg)) != -1) {
		if (key > 5)
			ck_abort_msg("Unknown key: %d\n", key);

		if (test_data[key].found)
			ck_abort_msg("Key read twice: %d\n", key);

		ck_assert_int_eq(key, test_data[key].key);
		test_data[key].found = 1;
		cnt -= 1;
	}

	ck_assert_int_eq(cnt, 0); // check that 5 keys were read
	ck_assert_int_eq(seg->next_bucket, 0);

	memtable_free(seg->table);
	free(seg);
} END_TEST


/*
 * Creates and returns a test suite for segmet_file functions
 */
Suite *segment_file_suite(void)
{
	Suite *s;
	TCase *tc;

	s = suite_create("Segment File");
	tc = tcase_create("Core");

	tcase_add_test(tc, test_segf_init);
	tcase_add_test(tc, test_segf_link_before);
	tcase_add_test(tc, test_segf_next_key);
	/* Add future segment_file struct test cases */
	
	suite_add_tcase(s, tc);
	return s;
}


START_TEST(test_segf_create_file)
{
	extern int segf_create_file(struct segment_file*);	

	struct segment_file seg;
	seg.size = 0;
	seg.seg_fd = -1;
	seg.name = "test.dat";
	seg.table = NULL;
	
	if (segf_create_file(&seg) < 0)
		ck_abort_msg("ERROR: Could not create segment file\n");

	int err = access(seg.name, F_OK);
	if (err == -1 && errno == ENOENT) {
		ck_abort_msg("ERROR: segf_create_file did not create '%s'\n",
				seg.name);
	} else if (err == -1 && errno != 0) {
		ck_abort_msg("ERROR: test_segf_create_file: %s", 
				strerror(errno));	
	}

	ck_assert_int_ne(seg.seg_fd, -1);
} END_TEST


START_TEST(test_segf_rename_file)
{
	extern int segf_rename_file(struct segment_file*, char*);

	char *name = "test.dat";
	char *first_name = calloc(strlen(name), sizeof(char));
	if (first_name == NULL)
		ck_abort_msg("ERROR: test_segf_rename_file: calloc fail\n");
	strncpy(first_name, name, strlen(name));

	struct segment_file seg;
	seg.name = first_name;	

	if (segf_rename_file(&seg, "test2.dat") < 0)
		ck_abort_msg("ERROR: rename fail\n");

	ck_assert_str_eq(seg.name, "test2.dat");
		
	int err = access(seg.name, F_OK);
	if (err == -1)
		ck_abort_msg("ERROR: file name not changed\n");

	free(seg.name);
} END_TEST


START_TEST(test_segf_delete_file)
{
	extern int segf_delete_file(struct segment_file*);	
	
	struct segment_file seg;
	seg.seg_fd = -1;
	seg.name = "test2.dat";
	seg.table = NULL;

	if (segf_delete_file(&seg) < 0)
		ck_abort_msg("ERROR: Could not delete segment file\n");
	
	ck_assert_int_eq(seg.seg_fd, -1);
	ck_assert_int_eq(seg.size, 0);

	// try and access the file
	int err = access(seg.name, F_OK);
	if (err == 0)
		ck_abort_msg("ERROR: '%s' was not deleted\n", seg.name);
} END_TEST


/*
 * Creates and returns a test suite for segment_file IO functions
 */
Suite *segment_file_io_suite(void)
{
	Suite *s;
	TCase *tc;

	s = suite_create("Segment File IO");
	tc = tcase_create("Core");
	
	tcase_add_test(tc, test_segf_create_file);
	tcase_add_test(tc, test_segf_rename_file);
	tcase_add_test(tc, test_segf_delete_file);

	suite_add_tcase(s, tc);
	return s;
}


int main(void)
{
	int fail = 0;
	Suite *s, *s2;
	SRunner *runner;
	
	s = segment_file_suite();
	s2 = segment_file_io_suite();
	runner = srunner_create(s);
	srunner_add_suite(runner, s2);

	srunner_run_all(runner, CK_NORMAL);
	fail = srunner_ntests_failed(runner);
	srunner_free(runner);

	return (fail == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
