#ifndef _TEST_PIPELINE_STAGE_H_
#define _TEST_PIPELINE_STAGE_H_

struct kv_pair {
	int key;
	char *value;
};

struct kv_pairs {
	int len;	
	struct kv_pair *data;
};

struct stage {
	const char *name;
	int seq_num;
	struct kv_pairs *data;
	struct stage *next;
	int (*run)(struct stage *this);
};

struct stage *stage_init(const char *n, int seq_num);

void stage_free(struct stage *s);

#endif
