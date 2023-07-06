#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hashtable.h"

static uint64_t djb_hash(const char *, uint64_t);
static void destroy_user(user_t ** pp_user);
static void print_table(hash_table_t *);

/*
 * @brief Creates a hash table and return a pointer to it.
 *
 * @return hash_table_t * on success; NULL on failure.
 */
hash_table_t *
create_table (void)
{
    hash_table_t * p_table = calloc(1, sizeof(hash_table_t));

    if (!p_table)
    {
        fprintf(stderr, "error: hash table creation failed\n");
        return NULL;
    }

    p_table->capacity = INIT_TBL_CAP;
    p_table->items    = calloc(p_table->capacity, sizeof(user_t *));

    if (!p_table->items)
    {
        fprintf(stderr, "error: hash table creation failed\n");
        return NULL;
    }

    return p_table;
} /* create_table */


/*** end of file ***/
