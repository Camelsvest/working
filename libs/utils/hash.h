#ifndef _HASH_H_
#define _HASH_H_

#include "linux-like-list/list.h"

#define INVALID_HASH_KEY			0xFFFFFFFF
#define HASH_BUCKET_SIZE(hash, key) hash->buckets[key].list_size

typedef struct _hash_entry hash_entry;
struct _hash_entry
{
	struct list_head list;
	unsigned int     list_size;
};

typedef unsigned int hash_key_t;

typedef struct _hash_t hash_t;
struct _hash_t {
        unsigned int        bucket_size;

        unsigned int        down_shift;
        unsigned int        mask;

        hash_entry  		*buckets;
        unsigned int        available_num;
};

typedef struct _hash_node_t hash_node_t;
struct _hash_node_t {
	struct list_head list;
	void *data;
};

#ifdef __cplusplus
extern "C" {
#endif

hash_t* hash_init();
void hash_uninit(hash_t *hash_table);

hash_key_t hash_lookup(hash_t *hash_table, void *data);
hash_key_t hash_insert(hash_t *hash_table, void *data);
hash_key_t hash_remove(hash_t *hash_table, void *data);
unsigned int hash_amount(hash_t *hash_table);

typedef void (*hash_visit_custom)(unsigned int key, void *data);
void hash_foreach(hash_t *table, hash_visit_custom func, void *data);

#ifdef __cplusplus
}
#endif

#endif
