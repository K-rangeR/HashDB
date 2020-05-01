/*
 * Tests for hashDB.c
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
	Suite *s;
	SRunner *runner;

	s = util_suite();
	runner = srunner_create(s);

	srunner_run_all(runner, CK_NORMAL);
	fail = srunner_ntests_failed(runner);
	srunner_free(runner);

	return (fail == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
