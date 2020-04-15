#include <stdio.h>
#include <stdlib.h>
#include "pipeline.h"


/*
 * Creates a new empty pipeline
 *
 * Params:
 *	none
 *
 * Returns:
 *	Dynamically allocated pipeline struct or NULL if there is no
 *	memory available
 */
struct pipeline *pl_init()
{
	struct pipeline *pl = NULL;	

	if ((pl = malloc(sizeof(struct pipeline))) == NULL)
		return NULL;
	pl->stages = 0;
	pl->first = NULL;
	return pl;
}


/*
 * Frees the given pipeline struct and all of its stages
 *
 * Params:
 *	pl => pipeline struct to free
 *
 * Returns:
 *	void
 */
void pl_free(struct pipeline *pl)
{
	struct stage *prev;

	prev = pl->first;
	while (pl->first) {
		pl->first = pl->first->next;
		free(prev);
		prev = pl->first;
	}

	free(pl);
}


/*
 * Builds a new testing pipeline using the stage info found the given
 * stage file.
 *
 * Params:
 *	pl => pointer to an already initialized pipeline struct	
 *	stage_file => name of the stage file to parse
 *
 * Returns:
 *	0 if successful (stage is ready to run), or -1 if there is an error
 */
int pl_parse_stage_file(struct pipeline *pl, const char *stage_file)
{
	struct stage *prev = NULL;
	for (int i = 0; i < 5; ++i) {
		struct stage *new_stage = stage_init("test_me", i);
		if (new_stage == NULL)
			return -1;
		if (prev == NULL)
			pl->first = new_stage;
		else
			prev->next = new_stage;
		prev = new_stage;
	}
	return 0;
}


/*
 * Runs the testing pipeline
 *
 * Params:
 *	pl => pointer to the pipeline struct to run
 *
 * Returns:
 *	1 if all stages pass, 0 if one fails (stages after failed stage are
 *	not run)
 */
int pl_run(struct pipeline *pl)
{
	struct stage *curr_stage = pl->first;
	while (curr_stage) {
		printf("%s | %d\n", curr_stage->name, curr_stage->seq_num);
		curr_stage = curr_stage->next;
	}
	return 1;
}
