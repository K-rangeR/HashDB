# HashDB
## Description
### Key Components
* Segment File: append only file that stores key value pairs
* Memory Table (memtable): in memory hash table that maps keys to value offsets in the associated segment file
