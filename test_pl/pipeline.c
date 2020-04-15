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
 *	none, but the pl pointer is set to NULL
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
	pl = NULL;
}
