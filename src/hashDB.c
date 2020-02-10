#include <dirent.h>
#include <errno.h>
#include "hashDB.h"

/*
 * Creates a hashDB struct that represents an active database. If data_dir
 * is the name of a directory with segment files in it, a linked list of
 * segment_file structs is created. The list is ordered in descending order 
 * with respect to the segment file names which represent ID's. If data_dir
 * does not exist then it is created and an empty segment file is added to it.
 *
 * Params:
 *	data_dir => name of a directory to read from or create
 *
 * Returns:
 *	Dynamically allocated hashDB struct that is ready to handle read and
 *	write requests. This struct must be deallocated using hashDB_free. If
 *	there is an error NULL is returned.
 */
struct hashDB *hashDB_init(const char *data_dir)
{
	struct hashDB *db;

	// check if the dir exists
	// if it does call the repopulate funtion
	// if it does call the create empty dir function
	// return 

	return db;
}
