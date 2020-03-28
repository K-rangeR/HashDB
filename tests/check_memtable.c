/*
 * Tests for memtable.c
 */

#include <check.h>
#include <stdlib.h>
#include "../src/memtable.h"


START_TEST(test_memte_init)
{
	extern struct memtable_entry *memte_init(int, unsigned int);
	extern   void memte_free(struct memtable_entry*);
	
	int tkey = 1;
	unsigned int toffset = 10;
	struct memtable_entry *e;

	if ((e = memte_init(tkey, toffset)) == NULL)
		ck_abort_msg("Could not create memtable_entry\n");

	ck_assert_int_eq(e->key, tkey);
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
	/* Future memtable_entry test cases */

	suite_add_tcase(s, tc);
	return s;
}


START_TEST(test_memtable_init)
{
	extern struct memtable *memtable_init();
	extern   void memtable_free(struct memtable*);

	struct memtable *tbl;
	if ((tbl = memtable_init()) == NULL)
		ck_abort_msg("Could not create memtable\n");
	
	ck_assert_uint_eq(tbl->entries, 0);
	ck_assert_ptr_nonnull(tbl->table);

	memtable_free(tbl);
} END_TEST


START_TEST(test_memtable_read_write)
{
	extern struct memtable *memtable_init();
	extern   void memtable_free(struct memtable*);
	extern    int memtable_write(struct memtable*, int, unsigned int);
	extern    int memtable_read(struct memtable*, int, unsigned int*);

	struct memtable *tbl;
	if ((tbl = memtable_init()) == NULL)
		ck_abort_msg("Could not create memtable\n");

	const int kv_pairs = 50;
	
	// add to the db
	unsigned int offset;
	for (int key = 1; key < kv_pairs; key++) {
		offset = key + 1;
		if (memtable_write(tbl, key, offset) < 0)
			ck_abort_msg("Could not insert into tbl\n");
		ck_assert_int_eq(key, tbl->entries);
	}

	// check the read
	for (int key = 1; key < kv_pairs; key++) {
		memtable_read(tbl, key, &offset);
		ck_assert_uint_eq(offset, key+1);
	}

	memtable_free(tbl);
} END_TEST


START_TEST(test_memtable_write_with_update)
{
	extern struct memtable *memtable_init();	
	extern   void memtable_free(struct memtable*);
	extern    int memtable_write(struct memtable*, int, unsigned int);
	extern    int memtable_read(struct memtable*, int, unsigned int*);

	struct memtable *tbl;
	if ((tbl = memtable_init()) == NULL)
		ck_abort_msg("Could not create memtable\n");

	const int kv_pairs = 5;
	unsigned int offset = 0;
	for (int i = 0; i < kv_pairs; ++i) {
		if (memtable_write(tbl, i, offset) < 0)
			ck_abort_msg("Could not write to memtable\n");
		offset += 1;
	}

	ck_assert_uint_eq(tbl->entries, kv_pairs);

	// update some of the offsets
	offset = 1;
	for (int i = 0; i < kv_pairs; i += 2) {
		if (memtable_write(tbl, i, offset) < 0)
			ck_abort_msg("Could not write to memtable\n");
		offset += 1;
	}

	ck_assert_uint_eq(tbl->entries, kv_pairs);
	
	offset = 1;
	unsigned int returned_offset = 0;
	for (int i = 0; i < kv_pairs; i += 2) {
		if (memtable_read(tbl, i, &returned_offset) == 0)
			ck_abort_msg("Could not read memtable\n");	
		ck_assert_uint_eq(returned_offset, offset);
		offset += 1;
	}

	memtable_free(tbl);	
} END_TEST


START_TEST(test_memtable_remove)
{
	extern struct memtable *memtable_init();
	extern   void memtable_free(struct memtable*);
	extern    int memtable_write(struct memtable*, int, unsigned int);
	extern    int memtable_read(struct memtable*, int, unsigned int*);

	struct memtable *tbl;
	if ((tbl = memtable_init()) == NULL)
		ck_abort_msg("Could not create memtable\n");

	// add testing data
	const int kv_pairs = 5;
	unsigned int offset;
	for (int key = 1; key < kv_pairs; key++) {
		offset = key + 1;		
		if (memtable_write(tbl, key, offset) < 0)
			ck_abort_msg("Could not insert into tbl\n");
	}

	// delete all kv pairs one by one from tbl
	int key_found;
	for (int key = 1; key < kv_pairs; key++) {
		if (memtable_remove(tbl, key) == 0)
			ck_abort_msg("Could not find '%d' to delete\n", key);

		// attempt to read the deleted kv pair
		if (memtable_read(tbl, key, &offset) != 0)
			ck_abort_msg("(%d,%d) was not deleted\n", key, offset);
	}

	ck_assert_uint_eq(tbl->entries, 0);
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
	tcase_add_test(tc, test_memtable_write_with_update);
	tcase_add_test(tc, test_memtable_remove);
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
