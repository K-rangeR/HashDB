#include <stdlib.h>
#include <string.h>
#include "stage.h"
#include "test_stages.h"


static void set_run_function(struct stage *);
static int init_kv_pair_array(struct stage *);


/*
 * Creates a new stage
 *
 * Params:
 *	name => pointer to a name to give the stage
 *	seq_num => sequence number of the stage relative to other stages
 *	data_count => number of testing key value pairs for this stage
 *
 * Returns:
 *	Pointer to a new stage struct or NULL if there is no memory available
 */
struct stage *stage_init(char *name, int seq_num, int data_count)
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
	new_stage->test_data.len = data_count;
	if (init_kv_pair_array(new_stage) < 0) {
		free(new_stage->name);
		free(new_stage);
		return NULL;
	}
	new_stage->next = NULL;
	set_run_function(new_stage);
	return new_stage;
}


/*
 * Uses the stages name to set the run function
 * to the appropriate test stage function
 * 
 * Params:
 *	s => stage struct to update
 *
 * Returns:
 *	void
 */
static void set_run_function(struct stage *s)
{
	s->run = test_nothing;
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
	// TODO: free test_data array and the values in it
	free(s);
}


static int init_kv_pair_array(struct stage *s)
{
	if (s->test_data.len == 0)
		return 1;

	s->test_data.data = calloc(s->test_data.len, sizeof(struct kv_pair));
	return (s->test_data.data != NULL) ? 0 : -1;
}
