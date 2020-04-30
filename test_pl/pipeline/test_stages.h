#ifndef _TEST_PIPELINE_TEST_STAGES_H_
#define _TEST_PIPELINE_TEST_STAGES_H_

#include "stage.h"

int test_segf_put(struct stage *s);
int test_segf_delete(struct stage *s);

int test_hashdb_put(struct stage *s);
int test_hashdb_delete(struct stage *s);
int test_hashdb_repopulate(struct stage *s);
int test_hashdb_mkempty(struct stage *s);
int test_hashdb_compact(struct stage *s);
int test_hashdb_merge(struct stage *s);

int test_nothing(struct stage *s);

#endif
