#ifndef _TEST_DATA_H_
#define _TEST_DATA_H_


/*
 * Testing data and info that can be used by testing code
 */

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


char *test_file_namesxl[] = {
	"tdata/hashDB_tdata/4.dat",
	"tdata/hashDB_tdata/3.dat",
	"tdata/hashDB_tdata/2.dat",
	"tdata/hashDB_tdata/1.dat"
};


test_kv tdxl[] = {
	{1, "one"},
	{2, "two"},
	{3, "three"},
	{4, "four"},
	{5, "five"},
	{6, "six"},
	{7, "seven"},
	{8, "eight"},
	{9, "nine"},
	{10, "ten"},
	{11, "eleven"},
	{12, "twelve"}
};


#define TOTAL_TEST_FILES 4
#define KV_PAIRS_PER_FILE 3
#define TOTAL_KV_PAIRS 5
#define TOTAL_KVXL_PAIRS 12
#define FILE_SIZE (sizeof(char) * 5) + (sizeof(int) * 15) + 24

#endif
