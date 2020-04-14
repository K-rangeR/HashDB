#ifndef _TEST_PIPELINE_PL_H_
#define _TEST_PIPELINE_PL_H_

#include "stage.h"

struct pipeline {
	int stages;	
	struct stage *first;
};

struct pipeline *pl_init();

int pl_parse_stage_file(struct pipeline *pl, const char *file);

int pl_parse_data_section(struct pipeline *pl, 
                          struct stage *curr_stage, 
                          int fd,
                          int n);

int pl_run(struct pipeline *pl);

#endif
