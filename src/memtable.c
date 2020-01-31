#include <stdlib.h>
#include "memtable.h"

/*
 * Creates a memtable_entry. Caller is responsible for freeing this struct
 * by calling the memte_free function.
 * 
 * Params:
 *	offset => offset in the segment file where a value is stored
 * 
 * Returns:
 *	Pointer to a struct memtable_entry on the heap if allocation was
 *	successful, NULL otherwise
 */
struct memtable_entry *memte_init(unsigned int offset)
{
	struct memtable_entry *e;

	if ((e = malloc(sizeof(struct memtable_entry))) == NULL)
		return NULL;

	e->offset = offset;
	e->next = NULL;
	return e;
}

/*
 * Frees all memory allocated for the given memtable entry ONLY.
 * 
 * Params:
 *	entry => pointer to the memtable entry to free
 * 
 * Returns:
 *	void
 */
void memte_free(struct memtable_entry *entry)
{
	free(entry);
}
