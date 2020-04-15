#include <stdio.h>
#include "test_stages.h"


/*
 * Does not perform any tests, just for debugging
 */
int test_nothing(struct stage *s)
{
	printf("test %d is testing nothing\n", s->seq_num);	
	return 1;
}
