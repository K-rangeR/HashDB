#ifndef _TEST_PIPELINE_PL_H_
#define _TEST_PIPELINE_PL_H_

#include "stage.h"

struct pipeline {
	int stages;	
	struct stage *first;
  struct stage *last;
};

struct pipeline *pl_init();

void pl_free(struct pipeline *pl);

int pl_parse_stage_file(struct pipeline *pl, const char *file);

int pl_run(struct pipeline *pl);

#endif
