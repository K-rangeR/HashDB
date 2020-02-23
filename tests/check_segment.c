/* 
 * Unit tests for segment.c
 */
#include <check.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include "../src/segment.h"
#include "data.h"

START_TEST(test_segf_init)
{
	extern struct segment_file *segf_init(char*);
	extern void segf_free(struct segment_file*);

	struct segment_file *seg;
	char *tname = malloc(sizeof(char) * 9);
	if (tname == NULL)
		ck_abort_msg("Could not malloc space for file name\n");
	strcpy(tname, "test.dat");

	if ((seg = segf_init(tname)) == NULL)
		ck_abort_msg("Could not create segment file struct\n");
	
	ck_assert_int_eq(seg->size, 0);
	ck_assert_str_eq(seg->name, tname);
	ck_assert_int_eq(seg->seg_fd, -1);
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

START_TEST(test_segf_unlink_front)
{
	extern void segf_unlink(struct segment_file*, struct segment_file*);

	// make a list of segment_files
	struct segment_file s1, s2, s3;
	s3.name = "s2.dat";
	s3.next = NULL;

	s2.name = "s2.dat";
	s2.next = &s3;

	s1.name = "s1.dat";
	s1.next = &s2;

	segf_unlink(&s1, &s1);
	ck_assert_ptr_null(s1.next);
	ck_assert_ptr_eq(s2.next, &s3);
	ck_assert_ptr_null(s3.next);
} END_TEST

START_TEST(test_segf_unlink_mid)
{
	extern void segf_unlink(struct segment_file*, struct segment_file*);

	// make a list of segment_files
	struct segment_file s1, s2, s3;
	s3.name = "s2.dat";
	s3.next = NULL;

	s2.name = "s2.dat";
	s2.next = &s3;

	s1.name = "s1.dat";
	s1.next = &s2;
	
	segf_unlink(&s1, &s2);
	ck_assert_ptr_null(s2.next);
	ck_assert_ptr_eq(s1.next, &s3);
} END_TEST

START_TEST(test_segf_unlink_last)
{
	extern void segf_unlink(struct segment_file*, struct segment_file*);

	// make a list of segment_files
	struct segment_file s1, s2, s3;
	s3.name = "s2.dat";
	s3.next = NULL;

	s2.name = "s2.dat";
	s2.next = &s3;

	s1.name = "s1.dat";
	s1.next = &s2;
	
	segf_unlink(&s1, &s3);
	ck_assert_ptr_null(s3.next);
	ck_assert_ptr_null(s2.next);
	ck_assert_ptr_eq(s1.next, &s2);
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
	tcase_add_test(tc, test_segf_unlink_front);
	tcase_add_test(tc, test_segf_unlink_mid);
	tcase_add_test(tc, test_segf_unlink_last);
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

	int err;
	err = access(seg.name, F_OK);
	if (err == -1 && errno == ENOENT) {
		ck_abort_msg("ERROR: segf_create_file did not create '%s'\n",
				seg.name);
	} else if (err == -1 && errno != 0) {
		ck_abort_msg("ERROR: test_segf_create_file: %s", 
				strerror(errno));	
	}

	ck_assert_int_ne(seg.seg_fd, -1);
} END_TEST

START_TEST(test_segf_delete_file)
{
	extern int segf_delete_file(struct segment_file*);	
	
	struct segment_file seg;
	seg.seg_fd = -1;
	seg.name = "test.dat";
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

START_TEST(test_segf_read_append)
{
	extern struct segment_file *segf_init(char*);
	extern void segf_free(struct segment_file*);
	extern int segf_read_file(struct segment_file*, int, char**);	
	extern int segf_append(struct segment_file*, int, char*, char);
	extern int segf_delete_file(struct segment_file*);	
	extern int segf_create_file(struct segment_file*);	

	char *tname = malloc(sizeof(char) * 9);
	if (tname == NULL)
		ck_abort_msg("Could not malloc space for file name\n");
	strcpy(tname, "test.dat");

	struct segment_file *seg;	
	if ((seg = segf_init(tname)) < 0)
		ck_abort_msg("ERROR: could not create segment struct\n");
	
	if (segf_create_file(seg) < 0)
		ck_abort_msg("ERROR: could not create file\n");
	
	// pointers used during the read part of the tests
	// will point to the values read from the file
	char *t = "xyz";
	char *test_ptrs[5] = {t, t, t, t, t};

	// test append
	for (int i = 0; i < TOTAL_KV_PAIRS; ++i) {
		if (segf_append(seg, td[i].key, td[i].val, 0) < 0)
			ck_abort_msg("ERROR: could not append to file\n");
	}

	// test read
	for (int i = 0; i < TOTAL_KV_PAIRS; ++i) {
		if (segf_read_file(seg, i+1, &test_ptrs[i]) <= 0)
			ck_abort_msg("ERROR: could not read from file\n");
		ck_assert_str_eq(test_ptrs[i], td[i].val);
	}

	// free the values read from the file
	for (int i = 0; i < TOTAL_KV_PAIRS; ++i)
		free(test_ptrs[i]);

	if (segf_delete_file(seg) < 0)
		ck_abort_msg("ERROR: could not delete file\n");	
	
	segf_free(seg);
} END_TEST

START_TEST(test_segf_remove_pair)
{
	extern struct segment_file *segf_init(char*);
	extern void segf_free(struct segment_file*);
	extern int segf_append(struct segment_file*, int, char*, char);
	extern int segf_delete_file(struct segment_file*);	
	extern int segf_create_file(struct segment_file*);	
	extern int segf_remove_pair(struct segment_file*, int);

	char *tname = malloc(sizeof(char) * 9);
	if (tname == NULL)
		ck_abort_msg("Could not malloc space for file name\n");
	strcpy(tname, "test.dat");

	struct segment_file *seg;	
	if ((seg = segf_init(tname)) < 0)
		ck_abort_msg("ERROR: could not create segment struct\n");
	
	if (segf_create_file(seg) < 0)
		ck_abort_msg("ERROR: could not create file\n");
	
	// add the testing data
	for (int i = 0; i < TOTAL_KV_PAIRS; ++i) {
		if (segf_append(seg, td[i].key, td[i].val, 0) < 0)
			ck_abort_msg("ERROR: could not append to file\n");
	}

	unsigned int offset;
	int err;
	for (int i = 0; i < TOTAL_KV_PAIRS; ++i) {
		if ((err = segf_remove_pair(seg, td[i].key)) <= 0) {
			if (err == 0)
				ck_abort_msg("ERROR: key not found\n");
			else
				ck_abort_msg("ERROR: delete failed\n");
		}

		// verify the key is not in the memtable
		if (segf_read_memtable(seg, td[i].key, &offset) == 1)
			ck_abort_msg("ERROR: key still in memtable\n");
	}

	// clean up
	if (segf_delete_file(seg) < 0)
		ck_abort_msg("ERROR: could not delete file\n");	
	
	segf_free(seg);
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
	tcase_add_test(tc, test_segf_delete_file);
	tcase_add_test(tc, test_segf_read_append);
	tcase_add_test(tc, test_segf_remove_pair);

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
