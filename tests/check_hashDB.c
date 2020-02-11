/*
 * Tests for hashDB.c
 */
#include <check.h>
#include <stdlib.h>
#include "../src/hashDB.h"

/*
 * Note if any of the tests in check_memtable and check_segment fail
 * this test will probably fail to
 */
START_TEST(test_hashDB_repopulate)
{
	extern struct hashDB * hashDB_repopulate(const char *data_dir);

	const char *test_dir = "tdata";

	hashDB_repopulate(test_dir);
} END_TEST

Suite *hashDB_suite(void)
{
	Suite *s;
	TCase *tc;

	s = suite_create("hashDB Repopulate");
	tc = tcase_create("Core");

	tcase_add_test(tc, test_hashDB_repopulate);

	suite_add_tcase(s, tc);
	return s;
}

int main(void)
{
	int fail = 0;
	Suite *s;
	SRunner *runner;

	s = hashDB_suite();
	runner = srunner_create(s);

	srunner_run_all(runner, CK_NORMAL);
	fail = srunner_ntests_failed(runner);
	srunner_free(runner);

	return (fail == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
