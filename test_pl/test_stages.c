#include <stdio.h>
#include "test_stages.h"


/*
 * Does not perform any tests, just for debugging
 */
int test_nothing(struct stage *s)
{
	printf("%s | len = %d | ID1 = %d | ID2 = %d\n", s->name, 
		test_data_len(s), s->segf_ids[0], s->segf_ids[1]);
	for (int i = 0; i < test_data_len(s); ++i) {
		printf("%d | %s\n", 
			key_at(s, i),
			value_at(s, i));
	}
	return 1;
}
