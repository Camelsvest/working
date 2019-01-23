#include <stdio.h>
#include "hash.h"

int main(int argc, char *argv[])
{
        int index;
        hash_t *table;
        hash_key_t key;

        table = hash_init();

        for (index = 0; index < 50000; index++)
        {
                key = hash_insert(table, (void *)index);
                printf("add key = %u, index = %d, bucket length: %u, \r\n",
                        key, index, HASH_BUCKET_SIZE(table, key));
        }

        printf("Hash finished now.\n");
        getchar();

        for (index = 0; index < 50000; index++)
        {
                printf("remove key = %u, index = %d, bucket length: %u\r\n",
                        key, index, HASH_BUCKET_SIZE(table, key));
                key = hash_remove(table, (void *)index);
        }

        hash_uninit(table);
        
        return 0;
}
