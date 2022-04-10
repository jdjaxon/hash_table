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


/*
 * @brief Creates a user.
 *
 * @param name: User's name.
 * @param name_len: Length of user's name.
 * @param passwd: User's password
 * @param pw_len: Length of user's password.
 * @param perms: User's permission level.
 * @return user_t * on success; NULL on failure.
 */
user_t *
create_user (const char * name,
             uint16_t name_len,
             const char * passwd,
             uint16_t pw_len,
             uint8_t perms)
{
    user_t * p_user = NULL;
    if (!name)
    {
        goto cleanup;
    }

    p_user = calloc(1, sizeof(user_t));

    if (!p_user)
    {
        goto cleanup;
    }

    p_user->name = calloc(name_len + 1, sizeof(char));
    memcpy(p_user->name, name, name_len);
    p_user->passwd = calloc(pw_len + 1, sizeof(char));
    memcpy(p_user->passwd, passwd, pw_len);
    p_user->perms = perms;

    return p_user;
cleanup:
    fprintf(stderr, "error: user creation failed\n");
    free(p_user);
    p_user = NULL;
    return p_user;
} /* create_user */


/*
 * @brief Inserts a user item into the hashtable.
 *
 * In the event of a collision, this algorithm inserts the new user at the
 * beginning of the chain of users, in an effort to maintain efficiency
 * upon insertion.
 *
 * @param pp_table: Double pointer to hashtable.
 * @param p_user: Pointer to user to be added.
 * @return true on success; false on failure.
 */
bool
insert_user (hash_table_t ** pp_table, user_t * p_user)
{
    if (!pp_table || !(*pp_table)->items || !p_user || !p_user->name)
    {
        fprintf(stderr, "error: unable to insert user\n");
        return false;
    }

    float new_lf = ((*pp_table)->num_items + 1) / (float)(*pp_table)->capacity;

    if (new_lf > LOAD_FACTOR)
    {
        (*pp_table) = rehash_table(pp_table);

        if (!(*pp_table))
        {
            return false;
        }
    }

    uint64_t res = djb_hash(p_user->name, strlen(p_user->name));

    if ((EINVAL == errno) && (0 == res))
    {
        perror("unable to hash username");
        return false;
    }

    uint64_t idx = res % (*pp_table)->capacity;
    p_user->next = (*pp_table)->items[idx];
    (*pp_table)->items[idx] = p_user;
    ++(*pp_table)->num_items;
    return true;
} /* insert_user */


/*** end of file ***/
