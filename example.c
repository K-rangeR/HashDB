#include <errno.h>
#include <stdio.h>
#include <string.h>
#include "src/hashDB.h" // only include needed

int main()
{
	struct hashDB *handler = NULL;
	if ((handler = hashDB_init("data_dir")) == NULL) {
		printf("No memory: %s\n", strerror(errno));
	}

	hashDB_free(handler);
	return 0;
}
