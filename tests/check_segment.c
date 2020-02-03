/* 
 * Unit tests for segment.c
 */
#include <check.h>
#include <stdlib.h>
#include "../src/segment.h"

START_TEST(test_segf_init)
{
	extern struct segment_file *segf_init(char*);
	extern void segf_free(struct segment_file*);

	struct segment_file *seg;
	char *tname = "test.dat";

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

int main(void)
{
	int fail = 0;
	Suite *s;
	SRunner *runner;
	
	s = segment_file_suite();
	runner = srunner_create(s);

	srunner_run_all(runner, CK_NORMAL);
	fail = srunner_ntests_failed(runner);
	srunner_free(runner);

	return (fail == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
