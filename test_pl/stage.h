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
	char *name;
	int seq_num;
	struct kv_pairs test_data;
	struct stage *next;
	int (*run)(struct stage *this);
};

#define insert_test_data(stage, data, index)             \
        stage->test_data.data[index] = data;

struct stage *stage_init(char *name, int seq_num, int data_count);

void stage_free(struct stage *s);

#endif
