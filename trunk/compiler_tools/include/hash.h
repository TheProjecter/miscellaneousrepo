#include <stdlib.h>
#include <math.h>

#define MAX_BUCKETS 4096

#ifndef VALUE_T
	#define VALUE_T
	#if __WORDSIZE == 64
		typedef	long long value_t;
	#else
		typedef int value_t;
	#endif
#endif

typedef struct bucket_t {
	char key[256];
	value_t value;
	struct bucket_t *next;
} bucket;

typedef struct hash_table_t {
	bucket buckets[MAX_BUCKETS];
} hash_table;

int hash_value(char* val) {
	int sum = 0;

	for (; *val; val++) {
		sum = 31*sum + *val;
	}

	return abs(sum) % MAX_BUCKETS;
}

void add_item(hash_table* ht, char* key, value_t value) {
	bucket *b = &ht->buckets[hash_value(key)];
	if (b->key[0]) {
		// already assigned, link new bucket
		for (; b->next; b=b->next);

		bucket *b2 = malloc(sizeof(bucket));
		strcpy(b2->key, key);
		b2->value = value;
		b2->next = NULL;
		b->next = b2;
	} else {
		// assign first bucket
		strcpy(b->key, key);
		b->value = value;
	}
}

value_t get_item(hash_table* ht, char* key) {
	bucket *b = &ht->buckets[hash_value(key)];
	for (; b; b=b->next) {
		if (!strcmp(key, b->key)) {
			return b->value;
		}
	}

	return -1; // not found
}

void init_hash_table(hash_table** ht) {
	int i;
	*ht = malloc(sizeof(hash_table));
	for (i = 0; i < MAX_BUCKETS; i++) {
		(*ht)->buckets[i].key[0] = 0;
		(*ht)->buckets[i].value = -1;
		(*ht)->buckets[i].next = NULL;
	}
}
