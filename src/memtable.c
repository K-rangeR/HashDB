#include <stdio.h>
#include <stdlib.h>
#include "memtable.h"

/*
 * Creates a memtable_entry. Caller is responsible for freeing this struct
 * by calling the memte_free function.
 * 
 * Params:
 *	key => identifies this entry in case of hash collisions
 *	offset => offset in the segment file where a value is stored
 * 
 * Returns:
 *	Pointer to a struct memtable_entry on the heap if allocation was
 *	successful, NULL otherwise
 */
struct memtable_entry *memte_init(int key, unsigned int offset)
{
	struct memtable_entry *e;

	if ((e = malloc(sizeof(struct memtable_entry))) == NULL)
		return NULL;

	e->key = key;
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

/*
 * Allocates an empty memtable of MAX_TBL_SZ
 *
 * Params:
 *	None
 *
 * Returns:
 *	Pointer to a memtable structure, caller must free struct
 *	memory used by calling memtable_free
 */
struct memtable *memtable_init()
{
	struct memtable *tbl;

	if ((tbl = malloc(sizeof(struct memtable))) == NULL)
		return NULL;

	tbl->entries = 0;

	// allocate empty table
	tbl->table = calloc(sizeof(struct memtable_entry*), MAX_TBL_SZ);
	if (tbl->table == NULL) {
		free(tbl);
		return NULL;
	}

	return tbl;
}

/*
 * Deallocates all memory used by the memtable, this includes the table and
 * bucket chains (memtable_entry linked list)
 *
 * Params:
 *	tbl => pointer the memtable to free
 * 
 * Returns:
 *	void
 */
void memtable_free(struct memtable *tbl)
{
	struct memtable_entry *curr, *prev;

	for (int i = 0; i < MAX_TBL_SZ; ++i) {
		if (tbl->table[i] == NULL) // empty bucket
			continue;

		// free the bucket chain
		curr = prev = tbl->table[i];
		while (curr) {
			curr = curr->next;
			memte_free(prev);
			prev = curr;
		}
	}

	free(tbl->table);
	free(tbl);
}

/*
 * Prints the entire memtable to stdout
 *
 * Params:
 *	tbl => pointer to the memtable to print
 *
 * Returns:
 *	void
 */
void memtable_dump(struct memtable *tbl)
{
	struct memtable_entry *e;

	for (int i = 0; i < MAX_TBL_SZ; ++i) {
		printf("Bucket: %d\n\t", i);
		e = tbl->table[i];
		while (e) {
			printf("%d %d -> ", e->key, e->offset);
			e = e->next;
		}
		printf("NULL\n");
	}
}

/*
 * Adds the offset to the memtable
 *
 * Params:
 *	tbl => pointer to the memtable to add to
 *	key => integer representing the key
 *	offset => offset in the segment file
 *
 * Returns:
 *	-1 if there is an error allocating memte_entry struct, 0 otherwise
 */
int memtable_write(struct memtable *tbl, int key, unsigned int offset)
{
	int hash = default_hash(key);
	hash = hash % MAX_TBL_SZ;

	struct memtable_entry *entry;
	if ((entry = memte_init(key, offset)) == NULL)
		return -1;
	
	// place new entry at the front of the bucket chain
	memte_place_before(entry, tbl->table[hash]);
	tbl->table[hash] = entry;
	tbl->entries += 1;
	return 0;
}

/*
 * Returns the offset of the given key in the memtable
 *
 * Params:
 *	tbl => pointer to the memtable to read from
 *	key => key of the target offset
 *	offset => address of where to store the offset it found
 *
 * Returns:
 *	1 if offset was found, 0 otherwise (does not change offset)
 */
int memtable_read(struct memtable *tbl, int key, unsigned int *offset)
{
	int hash = default_hash(key);
	hash = hash % MAX_TBL_SZ;

	struct memtable_entry *curr = tbl->table[hash];
	while (curr) {
		if (curr->key == key) {
			*offset = curr->offset;
			return 1;
		}
		curr = curr->next;
	}

	return 0;
}

/*
 * Default hash function for 32 bit integer keys
 * Taken from: https://gist.github.com/badboy/6267743
 *
 * Params:
 *	key => key to hash
 *
 * Returns:
 *	the hash value for the given key
 */
int default_hash(int key)
{
	uint32_t c2 = 0x27d4eb2d; // a prime or an odd constant
	key = (key ^ 61) ^ (key >> 16);
	key = key + (key << 3);
	key = key ^ (key >> 4);
	key = key * c2;
	key = key ^ (key >> 15);
	return key;
}
