#include <dirent.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include "pipeline.h"


// 'Private' helper functions mostly used for parsing the stage file
static struct stage *parse_section_header_line(char *line);
static struct stage *parse_segf_section_header(char *name, 
					       char *rest_of_line);
static struct stage *parse_hashdb_section_header(char *name, 
						 char *rest_of_line);
static struct kv_pair parse_data_section_line(char *line);
static void append_stage(struct pipeline *pl, struct stage *s);
static void nuke_test_data_dir();
static int delete_file(char*);


/*
 * Creates a new empty pipeline
 *
 * Params:
 *	none
 *
 * Returns:
 *	Dynamically allocated pipeline struct or NULL if there is no
 *	memory available
 */
struct pipeline *pl_init()
{
	struct pipeline *pl = NULL;	

	if ((pl = malloc(sizeof(struct pipeline))) == NULL)
		return NULL;
	pl->stages = 0;
	pl->first = NULL;
	pl->last = NULL;
	return pl;
}


/*
 * Frees the given pipeline struct and all of its stages
 *
 * Params:
 *	pl => pipeline struct to free
 *
 * Returns:
 *	void
 */
void pl_free(struct pipeline *pl)
{
	struct stage *prev;

	prev = pl->first;
	while (pl->first) {
		pl->first = pl->first->next;
		stage_free(prev);
		prev = pl->first;
	}

	free(pl);
}


/*
 * Builds a new testing pipeline using the stage info found the given
 * stage file.
 *
 * Params:
 *	pl => pointer to an already initialized pipeline struct	
 *	stage_file => name of the stage file to parse
 *
 * Returns:
 *	0 if successful (stage is ready to run), or -1 if there is an error
 */
int pl_parse_stage_file(struct pipeline *pl, const char *stage_file)
{
	FILE *fp;
	if ((fp = fopen(stage_file, "r")) == NULL)
		return -1;

	bool looking_for_section_header = true, in_data_section = false;
	struct stage *curr_stage = NULL;
	struct kv_pair data;
	int status = 0, next_kv = 0;
	char *line = NULL;
	size_t len = 0;
	ssize_t read;
	while ((read = getline(&line, &len, fp)) != -1) {
		if (line[0] == '#') // skip comment
			continue;
		
		if (looking_for_section_header) {
			curr_stage = parse_section_header_line(line);
			if (!curr_stage) {
				status = -1;
				break;
			}
			looking_for_section_header = false;
			in_data_section = true;
			next_kv = 0;
		} else if (strcmp(line, "\n") == 0) {
			append_stage(pl, curr_stage);
			looking_for_section_header = true;	
			in_data_section = false;
		} else if (in_data_section) {
			data = parse_data_section_line(line);
			insert_test_data(curr_stage, data, next_kv);
			next_kv++;
		}
	}
	append_stage(pl, curr_stage); // deal with final stage

	fclose(fp);
	if (line)
		free(line);
	return status;
}


/*
 * Converts the given stage section header from the stage file into the
 * appropriate stage struct.
 *
 * Params:
 *	line => stage header line to parse
 *
 * Returns:
 *	stage struct that contains the data from line if the line was in
 *	the correct format, or NULL otherwise
 */
static struct stage *parse_section_header_line(char *line)
{
	char *token = NULL, *string = line;
	struct stage *new_stage = NULL;

	token = strsep(&string, " ");
	if (token[0] == 's')
		new_stage = parse_segf_section_header(token, string);
	else if (token[0] == 'h')
		new_stage = parse_hashdb_section_header(token, string);
	else
		printf("[!] Unknown stage name: %s\n", token);

	return new_stage;
}


/*
 * Parses stage headers that begin with segf_*.
 *
 * Params:
 *	name => name of the segf stage (first token on the line)
 *	rest_of_line => un-parsed string from the stage header
 *
 * Returns:
 *	stage struct that contains the data from line if the line was in
 *	the correct format, or NULL otherwise
 */
static struct stage *parse_segf_section_header(char *name, 
					       char *rest_of_line)
{
	char *token = NULL, *argv[2];
	int argc = 0;

	while ((token = strsep(&rest_of_line, " ")) != NULL)
		argv[argc++] = token;	

	if (argc != 2) {
		printf("[!] segf_* stage header is incorrect\n");
		printf("\tUsage: segf_* [file_id] [number_of_kv_pairs]\n");
		return NULL;
	}

	int id = atoi(argv[0]);
	int kv_pair_count = atoi(argv[1]);
	struct stage *s = stage_init(name, kv_pair_count, 1, id);
	return s;
}


/*
 * Parses stage headers that begin with hashdb_*.
 *
 * Params:
 *	name => name of the segf stage (first token on the line)
 *	rest_of_line => un-parsed string from the stage header
 *
 * Returns:
 *	stage struct that contains the data from line if the line was in
 *	the correct format, or NULL otherwise
 */
static struct stage *parse_hashdb_section_header(char *name, 
					         char *rest_of_line)
{
	char *token = NULL, *argv[2];
	int argc = 0;

	while ((token = strsep(&rest_of_line, " ")) != NULL)
		argv[argc++] = token;

	struct stage *s = NULL;
	if (argc == 0) {
		name[strcspn(name, "\n")] = 0; // strip newline
		s = stage_init(name, 0, 0);
	} else if (argc == 1) {
		int kv_pair_count = atoi(argv[0]);
		s = stage_init(name, kv_pair_count, 0);
	} else if (argc == 2) {
		int id1 = atoi(argv[0]);	
		int id2 = atoi(argv[1]);	
		s = stage_init(name, 0, 2, id1, id2);
	} else {
		printf("[!] hashdb_* stage header is incorrect\n");
	}

	return s;
}


/*
 * Parses lines in the file that contain a key and possibly a value.
 *
 * Param:
 *	line => line from the stage file to parse
 *
 * Returns:
 *	kv_pair struct containing the parsed data
 */
static struct kv_pair parse_data_section_line(char *line)
{
	struct kv_pair data = {0, NULL};
	char *argv[2];
	int argc = 0;

	while ((argv[argc] = strsep(&line, " ")) != NULL)
		argc++;
	
	data.key = atoi(argv[0]);
	if (argc == 2) {
		char *val = argv[1];
		val[strcspn(val, "\n")] = 0; // strip new line
		int n = strlen(val);
		data.value = calloc(n, sizeof(char));
		if (data.value == NULL)
			printf("[!] Error making space for value\n");
		strncpy(data.value, val, n);
	}

	return data;
}


/*
 * Appends the given stage struct to end of the pipelines list of stages
 */
static void append_stage(struct pipeline *pl, struct stage *s)
{
	if (pl->first == NULL) {
		pl->first = s;
		pl->last = s;
	} else {
		pl->last->next = s;	
		pl->last = s;
	}
}


/*
 * Runs the testing pipeline
 *
 * Params:
 *	pl => pointer to the pipeline struct to run
 *
 * Returns:
 *	1 if all stages pass, 0 if one fails (stages after failed stage are
 *	not run)
 */
int pl_run(struct pipeline *pl)
{
	struct stage *curr_stage = pl->first;
	while (curr_stage) {
		if (!curr_stage->run(curr_stage))
			return 0;
		printf("[P]: %s\n\n", curr_stage->name);
		curr_stage = curr_stage->next;
	}

	nuke_test_data_dir();
	return 1;
}


static void nuke_test_data_dir()
{
	DIR *tdata_dir = opendir(TEST_DATA_DIR);		
	if (tdata_dir == NULL) {
		printf("[!] Could not open %s: %s\n", 
			TEST_DATA_DIR, strerror(errno));
		return;
	}
	chdir(TEST_DATA_DIR);
	struct dirent *de;
	while ((de = readdir(tdata_dir)) != NULL) {
		if (delete_file(de->d_name) < 0)
			printf("[!] Could not remove '%s': %s\n", de->d_name,
				strerror(errno));
	}
	closedir(tdata_dir);
}


static int delete_file(char *name)
{
	if (strcmp(name, ".") != 0 && strcmp(name, "..") != 0)
		return remove(name);
	return 0;
}
