#include <stdlib.h>
#include <string.h>
#include "stage.h"


/*
 * Creates a new stage
 *
 * Params:
 *	name => pointer to a name to give the stage
 *	seq_num => sequence number of the stage relative to other stages
 *
 * Returns:
 *	Pointer to a new stage struct or NULL if there is no memory available
 */
struct stage *stage_init(char *name, int seq_num)
{
	struct stage *new_stage = NULL;
	
	if ((new_stage = malloc(sizeof(struct stage))) == NULL)
		return NULL;
	
	int name_len = strlen(name);
	if ((new_stage->name = calloc(name_len, sizeof(char))) == NULL) {
		free(new_stage);
		return NULL;
	}

	strncpy(new_stage->name, name, name_len);
	new_stage->seq_num = seq_num;
	new_stage->data = NULL;
	new_stage->next = NULL;
	new_stage->run = NULL;
	return new_stage;
}


/*
 * Frees the given stage struct
 *
 * Parameter:
 *	s => stage struct to free
 *
 * Returns:
 *	void
 */
void stage_free(struct stage *s)
{
	free(s->name);
	free(s);
}
