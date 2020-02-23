#ifndef _TEST_DATA_H_
#define _TEST_DATA_H_

/*
 * Testing data and info that can be used by testing code
 */

/*
 * Names of all segment files used to test hashDB functions on
 */
char *test_file_names[] = {
	"tdata/4.dat",
	"tdata/3.dat",
	"tdata/2.dat",
	"tdata/1.dat"
};

#define TOTAL_TEST_FILES 4

typedef struct test_kv {
	int key;
	char *val;
} test_kv;

/*
 * Testing data, this is added to the segment files in the data directory
 * each time the test suite runs, and is removed from the segment files
 * when the suite is done.
 */
test_kv td[] = {
	{1, "one"},
	{2, "two"},
	{3, "three"},
	{4, "four"},
	{5, "five"}
};

#define TOTAL_KV_PAIRS 5
#define FILE_SIZE (sizeof(char) * 5) + (sizeof(int) * 15) + 24

#endif
