#include <stdio.h>
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

/*
 * Places e1 before e2 in the list. e1 must not be null.
 *
 * Params:
 *	e1 => pointer to a memtable_entry to put before e2
 *	e2 => pointer to a memtable_entry that is not changed
 *
 * Returns:
 *	void
 */
void memte_place_before(struct memtable_entry *e1, struct memtable_entry *e2)
{
	if (e1 == NULL) { // do something better later
		fprintf(stderr, "memte_place_before: e1 was NULL\n");
		exit(1);
	}
	e1->next = e2;
}

/*
 * Removes e from the linked list. Sets its next ptr to NULL.
 *
 * Params:
 *	head => start of the linked list
 *	e => entry to remove
 *
 * Returns:
 *	void
 */
void memte_remove(struct memtable_entry *head, struct memtable_entry *e)
{
	struct memtable_entry *prev;

	if (head == e) {
		e->next = NULL;
		return;
	}

	prev = head;
	while (prev && prev->next != e)
		prev = head->next;
	
	prev->next = e->next;
	e->next = NULL;
}
