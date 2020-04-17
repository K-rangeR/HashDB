#include <stdio.h>
#include "test_stages.h"


/*
 * Does not perform any tests, just for debugging
 */
int test_nothing(struct stage *s)
{
	printf("%s | %d | %d\n", s->name, s->seq_num, s->data.len);
	return 1;
}
