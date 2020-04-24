#include <stdio.h>
#include "test_stages.h"
#include "../src/hashDB.h"


int test_segf_get(struct stage *s)
{
	printf("test_segf_get\n");
	return 1;	
}


int test_segf_put(struct stage *s)
{
	printf("test_segf_put\n");
	return 1;	
}


int test_segf_delete(struct stage *s)
{
	printf("test_segf_delete\n");
	return 1;	
}


int test_hashdb_get(struct stage *s)
{
	printf("test_hash_get\n");
	return 1;
}


int test_hashdb_put(struct stage *s)
{
	printf("test_hashdb_put\n");
	return 1;
}


int test_hashdb_delete(struct stage *s)
{
	printf("test_hashdb_delete\n");
	return 1;
}


int test_hashdb_repopulate(struct stage *s)
{
	printf("test_hashdb_repopulate\n");
	return 1;
}


int test_hashdb_mkempty(struct stage *s)
{
	printf("test_hashdb_mkempty\n");
	return 1;
}


int test_hashdb_compact(struct stage *s)
{
	printf("test_hashdb_compact\n");
	return 1;
}


int test_hashdb_merge(struct stage *s)
{
	printf("test_hashdb_merge\n");
	return 1;
}


/*
 * Does not perform any tests, just for debugging
 */
int test_nothing(struct stage *s)
{
	/*
	printf("%s | len = %d | ID1 = %d | ID2 = %d\n", s->name, 
		test_data_len(s), id_one(s), id_two(s));
	for (int i = 0; i < test_data_len(s); ++i) {
		printf("%d | %s\n", 
			key_at(s, i),
			value_at(s, i));
	}
	*/
	printf("Nothing to test for '%s', check spelling...\n", s->name);
	return 1;
}
