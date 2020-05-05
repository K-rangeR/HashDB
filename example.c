#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "src/hashDB.h" // only include needed

int main()
{
	struct hashDB *handler = NULL;
	if ((handler = hashDB_init("data_dir")) == NULL) {
		printf("No memory: %s\n", strerror(errno));
		return 1;
	}

	// Put
	int key = 1;
	char *value = "Hello world";
	int err = hashDB_put(handler, key, strlen(value), value);
	if (err < 0) {
		printf("ERROR: %s\n", strerror(errno));
		return 1;
	}

	// Get
	char *db_value; // will store the value (need to free this)
	err = hashDB_get(handler, key, &db_value);
	if (err == 1) {
		printf("Value found: %s\n", db_value);
		free(db_value);
	} else if (err == 0) {
		printf("Value was not found\n");
	} else {
		printf("ERROR: %s\n", strerror(errno));	
		return 1;
	}

	// Delete
	err = hashDB_delete(handler, key);
	if (err == 1) {
		printf("Key value pair was deleted\n");
	} else if (err == 0) {
		printf("Key value pair was not found\n");
	} else {
		printf("ERROR: %s\n", strerror(errno));
		return 1;
	}

	hashDB_free(handler);
	return 0;
}
