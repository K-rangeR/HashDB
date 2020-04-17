#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "pipeline.h"


static int parse_section_header_line(char *line);


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
	char *line = NULL;
	size_t len = 0;
	ssize_t read;
	while ((read = getline(&line, &len, fp)) != -1) {
		if (line[0] == '#' ) // skip comment
			continue;
		
		if (looking_for_section_header) {
			parse_section_header_line(line);
			looking_for_section_header = false;
			in_data_section = true;
		} else if (strcmp(line, "\n") == 0) {
			looking_for_section_header = true;	
			in_data_section = false;
		} else if (in_data_section) {
			printf("in data section\n");	
		}
	}

	fclose(fp);
	if (line)
		free(line);
	return 0;
}


static int parse_section_header_line(char *line)
{
	char *token = NULL, *string = line;
	
	while ((token = strsep(&string, " ")) != NULL) {
		printf("%s\n", token);
	}

	return 0;
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
		curr_stage = curr_stage->next;
	}
	return 1;
}
