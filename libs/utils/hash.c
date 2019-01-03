#include <assert.h>
#include <stdlib.h>
#include "hash.h"

static hash_key_t fnv_random_key(unsigned int index, unsigned int down_shift, unsigned int mask)
{
        return (unsigned int)(((index * 1103515245) >> down_shift) & mask);
}

static hash_key_t hash_pointer_key(hash_t *hash_table, void *data)
{
        return fnv_random_key((unsigned int)data, hash_table->down_shift, hash_table->mask);
}

hash_t* hash_init()
{
		unsigned int index;
        hash_t *hash_table = NULL;
		const unsigned int hash_size = 512;
		
        if (hash_size > 0)
        {
                hash_table = (hash_t *)calloc(1, sizeof(hash_t));
				if (hash_table)
				{
	                hash_table->down_shift = 22;
	                hash_table->mask = 0x1FF;
	                hash_table->buckets = (hash_entry *)calloc(hash_size, sizeof(hash_entry));
					if (hash_table->buckets != NULL)
					{
						hash_table->bucket_size = hash_size;
						hash_table->available_num = 0;

						for (index = 0; index < hash_size; index++)
							INIT_LIST_HEAD(&hash_table->buckets[index].list);
					}
					else
					{
						free(hash_table);
						hash_table = NULL;
					}
				}
        }

        return hash_table;
}

void hash_uninit(hash_t *hash_table)
{
        if (hash_table)
        {
                free(hash_table->buckets);
                free(hash_table);
        }
}

hash_key_t hash_lookup(hash_t *hash_table, void *data)
{
        unsigned int key = hash_pointer_key(hash_table, data);

        assert(key < hash_table->bucket_size);

        return key;
}

hash_key_t hash_insert(hash_t *hash_table, void *data)
{
        hash_node_t *node;
        unsigned int key = hash_lookup(hash_table, data);

		node = (hash_node_t *)malloc(sizeof(hash_node_t));
		if (node != NULL)
		{
			node->data = data;
			
			list_add(&node->list, &hash_table->buckets[key].list);
			hash_table->buckets[key].list_size++;
		}

        hash_table->available_num++;

        return key;
}

hash_key_t hash_remove(hash_t *hash_table, void *data)
{		
        hash_node_t *node;
	    struct list_head *pos, *next;
		unsigned int ret = INVALID_HASH_KEY;
        unsigned int key = hash_lookup(hash_table, data);

		list_for_each_safe(pos, next, &hash_table->buckets[key].list)
		{
			node = list_entry(pos, struct _hash_node_t, list);
			if (node->data == data)
			{
				list_del(&node->list);
		        hash_table->available_num--;
				hash_table->buckets[key].list_size--;

				ret = key;
			}
		}

        return ret;
}

unsigned int hash_amount(hash_t *hash_table)
{
        if (hash_table)
        {
                return hash_table->available_num;
        }
        else
                return 0;
}

void hash_foreach(hash_t *table, hash_visit_custom func, void *data)
{
        hash_node_t *node;
        unsigned int index;
		struct list_head *pos;

        for (index = 0; index < table->bucket_size; index++)
        {
                if (list_empty(&table->buckets[index].list) == 0)
                {					
                        list_for_each(pos, &table->buckets[index].list)
                        {
							node = list_entry(pos, struct _hash_node_t, list);
							func(index, node->data);
                        }
                }                               
        }
}
