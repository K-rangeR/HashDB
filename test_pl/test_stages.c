#include <stdio.h>
#include "test_stages.h"


/*
 * Does not perform any tests, just for debugging
 */
int test_nothing(struct stage *s)
{
	printf("%s | %d | %d\n", s->name, s->seq_num, s->test_data.len);
	for (int i = 0; i < s->test_data.len; ++i) {
		printf("%d | %s\n", 
			s->test_data.data[i].key,
			s->test_data.data[i].value);
	}
	return 1;
}
