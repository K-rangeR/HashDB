#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
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
struct stage *stage_init(char *name, int data_count, int id_cnt, ...)
{
	struct stage *new_stage = NULL;
	
	if ((new_stage = calloc(1, sizeof(struct stage))) == NULL)
		return NULL;
	
	int name_len = strlen(name);
	if ((new_stage->name = calloc(name_len, sizeof(char))) == NULL) {
		free(new_stage);
		return NULL;
	}
	strncpy(new_stage->name, name, name_len);

	new_stage->test_data.len = data_count;
	if (init_kv_pair_array(new_stage) < 0) {
		free(new_stage->name);
		free(new_stage);
		return NULL;
	}
	new_stage->next = NULL;

	new_stage->segf_ids[0] = 0;
	new_stage->segf_ids[1] = 0;

	int i = 0;
	va_list ids;
	va_start(ids, id_cnt);
	while (i < id_cnt) {
		new_stage->segf_ids[i] = va_arg(ids, int);	
		i++;
	}
	va_end(ids);

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
	for (int i = 0; i < test_data_len(s); ++i) {
		if (value_at(s, i))
			free(value_at(s, i));
	}
	free(test_data(s));
	free(s);
}


/*
 * Creates an array of kv_pair structs for the given stage
 *
 * Param:
 *	s => stage to create the array for
 *
 * Returns:
 *	0 if successful, -1 if out of memory
 */
static int init_kv_pair_array(struct stage *s)
{
	if (test_data_len(s) == 0)
		return 1;

	assign_test_data(s, calloc(test_data_len(s), sizeof(struct kv_pair)));
	return (test_data(s) != NULL) ? 0 : -1;
}
