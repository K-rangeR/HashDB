#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "pipeline.h"


static struct stage *parse_section_header_line(char *line);
static void parse_data_section_line(char *line);
static void append_stage(struct pipeline *pl, struct stage *s);


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
	struct stage *curr_stage = NULL;
	char *line = NULL;
	size_t len = 0;
	ssize_t read;
	while ((read = getline(&line, &len, fp)) != -1) {
		if (line[0] == '#' ) // skip comment
			continue;
		
		if (looking_for_section_header) {
			curr_stage = parse_section_header_line(line);
			if (!curr_stage)
				break; // TODO: set return code
			looking_for_section_header = false;
			in_data_section = true;
		} else if (strcmp(line, "\n") == 0) {
			append_stage(pl, curr_stage);
			looking_for_section_header = true;	
			in_data_section = false;
		} else if (in_data_section) {
			parse_data_section_line(line);
		}
	}
	append_stage(pl, curr_stage); // deal with final stage

	fclose(fp);
	if (line)
		free(line);
	return 0;
}


static struct stage *parse_section_header_line(char *line)
{
	char *token = NULL, *string = line;
	char *argv[3]; // name ID number_of_kv_pairs
	int argc = 0;

	while ((token = strsep(&string, " ")) != NULL)
		argv[argc++] = token;

	if (argc != 3) {
		printf("[!] Section header is missing info\n");
		return NULL;
	}

	// TODO: add error checking
	int id = atoi(argv[1]);
	int data_count = atoi(argv[2]);

	struct stage *new_stage = stage_init(argv[0], id, data_count);
	if (!new_stage) {
		printf("[!] Could not create stage\n");
		return NULL;
	}

	return new_stage;
}


static void parse_data_section_line(char *line)
{
	return;
}


static void append_stage(struct pipeline *pl, struct stage *s)
{
	if (pl->first == NULL) {
		pl->first = s;
		return;
	}

	struct stage *curr = pl->first;
	while (curr->next != NULL)
		curr = curr->next;
	
	curr->next = s;
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
