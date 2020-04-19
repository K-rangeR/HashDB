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
  int segf_ids[2];
	struct kv_pairs test_data;
	struct stage *next;
	int (*run)(struct stage *this);
};

#define insert_test_data(stage, data, index) \
        stage->test_data.data[index] = data

#define value_at(stage, index) \
        stage->test_data.data[index].value

#define test_data_len(stage) \
        stage->test_data.len

#define test_data(stage) \
        stage->test_data.data

#define assign_test_data(stage, td) \
        stage->test_data.data = td

struct stage *stage_init(char *name, int seq_num, int data_count);

void stage_free(struct stage *s);

#endif
