# HashDB
## Description
Database that stores key value pairs in append only files.

## Key Components
* Segment File: append only file that stores key value pairs
* Database: directory of segment files
* Memory Table (memtable): in memory hash table that maps keys to value offsets in the associated segment file
* hashDB: C struct that represents a database handler, any iteraction with the database is done through this handler

## Supported Operations
* Put(key, value): appends a new key value pair in the newest segment file (also used for updating existing key value pairs)
* Get(key): retrieves the most up to date value associated with the key
* Delete(key): deletes the key value pair from the database

## Storage Management
Because segment files are append only, updates and deletes are not done in place. Instead a new key value pair is appended to a segment file and the associated memtable is updated to reflect the change. This may cause stale data to persist in the database following one of those operations. To address this problem, segment files are compacted once they reach a particular size. The compaction algorithm will create a new segment file that contains only the most up to date key value pairs from the segment file that is being compacted. The old segment file is then deleted when the compaction is done. Doing it this way ensures that the data is not corrupted if the compaction fails.

The compact algorithm may result in many small segment files which could slow down the speed of searches. To address this problem a merge algorithm will detect when two segment files can be merged and will then merge them into one segment file.

## Building HashDB
Run the makefile at the root of the project directory. This will create a shared library that can be linked with any program that want to use the databases functionality.

For example:
```
$ make
$ gcc -o ex example.c hashDB.so
$ ./ex
```
