/*
 * Unit tests for memtable.c
 */

#include <check.h>
#include <stdlib.h>
#include "../src/memtable.h"

START_TEST(test_memte_init)
{
	extern struct memtable_entry *memte_init(int, int);
	extern void memte_free(struct memtable_entry*);
	
	struct memtable_entry *e;
	int tkey = 1;
	int toffset = 10;
	e = memte_init(tkey, toffset);
	if (e == NULL)
		ck_abort_msg("Could not create memtable_entry\n");
	ck_assert_uint_eq(e->key, tkey);
	ck_assert_uint_eq(e->offset, toffset);
	ck_assert_ptr_null(e->next);
	memte_free(e);
} END_TEST

START_TEST(test_memte_place_before)
{
	extern void memte_place_before(struct memtable_entry*, 
	                               struct memtable_entry*);

	struct memtable_entry e1;
	e1.offset = 100;
	e1.next = NULL;

	struct memtable_entry e2;
	e2.offset = 200;
	e2.next = NULL;

	memte_place_before(&e1, &e2);
	ck_assert_ptr_eq(e1.next, &e2);

	memte_place_before(&e1, NULL);
	ck_assert_ptr_null(e1.next);
} END_TEST

START_TEST(test_memte_remove_mid)
{
	extern void memte_remove(struct memtable_entry*, struct memtable_entry*);

	// make a list
	struct memtable_entry one, two, three;
	three.offset = 3;
	three.next = NULL;

	two.offset = 2;
	two.next = &three;

	one.offset = 1;
	one.next = &two;

	// remove two from the list
	memte_remove(&one, &two);

	ck_assert_ptr_null(two.next);
	ck_assert_ptr_eq(one.next, &three);
	ck_assert_ptr_null(three.next);
} END_TEST

START_TEST(test_memte_remove_front)
{
	extern void memte_remove(struct memtable_entry*, struct memtable_entry*);

	// make a list
	struct memtable_entry one, two, three;
	three.offset = 3;
	three.next = NULL;

	two.offset = 2;
	two.next = &three;

	one.offset = 1;
	one.next = &two;

	// remove one from the list
	memte_remove(&one, &one);
	ck_assert_ptr_null(one.next);
	ck_assert_ptr_eq(two.next, &three);
	ck_assert_ptr_null(three.next);
} END_TEST

START_TEST(test_memte_remove_last)
{
	extern void memte_remove(struct memtable_entry*, struct memtable_entry*);

	// make a list
	struct memtable_entry one, two, three;
	three.offset = 3;
	three.next = NULL;

	two.offset = 2;
	two.next = &three;

	one.offset = 1;
	one.next = &two;

	// remove three from the list
	memte_remove(&one, &three);
	ck_assert_ptr_null(two.next);
	ck_assert_ptr_null(three.next);
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
	tcase_add_test(tc, test_memte_place_before);
	tcase_add_test(tc, test_memte_remove_mid);
	tcase_add_test(tc, test_memte_remove_front);
	tcase_add_test(tc, test_memte_remove_last);
	/* Future memtable_entry test cases */

	suite_add_tcase(s, tc);
	return s;
}

START_TEST(test_memtable_init)
{
	extern struct memtable *memtable_init();
	extern void memtable_free(struct memtable*);

	struct memtable *tbl;
	tbl = memtable_init();
	if (tbl == NULL)
		ck_abort_msg("Could not create memtable\n");
	
	ck_assert_uint_eq(tbl->entries, 0);
	ck_assert_ptr_nonnull(tbl->table);
	memtable_free(tbl);
} END_TEST

START_TEST(test_memtable_read_write)
{
	extern struct memtable *memtable_init();
	extern void memtable_free(struct memtable*);
	extern int memtable_write(struct memtable*, int, int);
	extern int default_hash(int);

	struct memtable *tbl;
	tbl = memtable_init();
	if (tbl == NULL)
		ck_abort_msg("Could not create memtable\n");
	
	int key;
	// add to the db
	for (key = 1; key < 50; key++) {
		if (memtable_write(tbl, key, key+1) < 0)
			ck_abort_msg("Could not insert into tbl\n");
		ck_assert_int_eq(key, tbl->entries);
	}

	// check the read
	for (key = 1; key < 50; key++) {
		int offset = memtable_read(tbl, key);
		ck_assert_int_eq(offset, key+1);
	}

	memtable_free(tbl);
} END_TEST

/*
 * Creates and returns a test suite for memtable functions
 */
Suite *memtable_suite(void)
{
	Suite *s;
	TCase *tc;

	s = suite_create("Memtable");
	tc = tcase_create("Core");

	tcase_add_test(tc, test_memtable_init);
	tcase_add_test(tc, test_memtable_read_write);
	/* Future memtable test cases */

	suite_add_tcase(s, tc);
	return s;
}

int main(void)
{
	int fail = 0;
	Suite *s, *s2;

	SRunner *runner;

	s = memtable_entry_suite();
	s2 = memtable_suite();
	runner = srunner_create(s);
	srunner_add_suite(runner, s2);

	srunner_run_all(runner, CK_NORMAL);
	fail = srunner_ntests_failed(runner);
	srunner_free(runner);

	return (fail == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
