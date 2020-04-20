#include <stdio.h>
#include "test_stages.h"


/*
 * Does not perform any tests, just for debugging
 */
int test_nothing(struct stage *s)
{
	printf("%s | %d | %d\n", s->name, s->seq_num, test_data_len(s));
	for (int i = 0; i < test_data_len(s); ++i) {
		printf("%d | %s\n", 
			key_at(s, i),
			value_at(s, i));
	}
	return 1;
}
