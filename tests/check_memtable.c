/*
 * Unit tests for memtable.c
 */

#include <check.h>
#include <stdlib.h>
#include "../src/memtable.h"

START_TEST(test_memte_init)
{
	extern struct memtable_entry *memte_init(unsigned int);
	extern void memte_free(struct memtable_entry*);
	
	struct memtable_entry *e;
	unsigned int toffset = 10;
	e = memte_init(toffset);
	if (e == NULL)
		ck_abort_msg("Could not create memtable_entry\n");
	ck_assert_uint_eq(e->offset, toffset);
	ck_assert_ptr_null(e->next);
	memte_free(e);
} END_TEST

/*
 * Creates and returns a test suite for memtable entry functions
 */
Suite *memtable_entry_suite(void)
{
	Suite *s;
	TCase *tc;

	s = suite_create("Memtable Entry");
	tc = tcase_create("Core");

	tcase_add_test(tc, test_memte_init);
	/* Future memtable_entry test cases */

	suite_add_tcase(s, tc);
	return s;
}

int main(void)
{
	int fail = 0;
	Suite *s;

	SRunner *runner;

	s = memtable_entry_suite();
	runner = srunner_create(s);

	srunner_run_all(runner, CK_NORMAL);
	fail = srunner_ntests_failed(runner);
	srunner_free(runner);

	return (fail == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
