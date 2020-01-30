#ifndef _HASHDB_MEMTABLE_H_
#define _HASHDB_MEMTABLE_H_

// Represents an entry in the memtables bucket chain
struct memtable_entry {
	unsigned int offset;
	struct memtable_entry *next;
};

struct memtable_entry *memte_init(unsigned int offset);
void memte_free(struct memtable_entry *entry);
void memte_place_before(struct memtable_entry *e1, struct memtable_entry *e2);
void memte_remove(struct memtable_entry *entry);

// Represents a memtable (hash table) that maps a key to a values offset
// in a segment file
struct memtable {
	unsigned int entries
	struct memtable_entry *table[];
};

struct memtable *memtable_init();
void memtable_free(struct memtable *tbl);
int memtable_read(struct memtable *tbl, int key);
void memtable_write(struct memtable *tbl, int key, offset);

#endif
