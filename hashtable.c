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


/*
 * @brief Locates an entry in the table by name.
 *
 * @param p_table: Pointer to table.
 * @param name: Name of potential user.
 * @return user_t * upon success; NULL on failure.
 */
user_t *
lookup_user (hash_table_t * p_table, const char * name)
{
    if (!p_table || !p_table->items || !name)
    {
        fprintf(stderr, "error: lookup failed\n");
        return NULL;
    }

    uint64_t name_len = strlen(name);
    uint64_t idx = djb_hash(name, name_len) % p_table->capacity;

    if ((EINVAL == errno) && (0 == idx))
    {
        perror("unable to hash username.");
        return false;
    }

    user_t * p_temp = p_table->items[idx];

    while (p_temp)
    {
        if (!p_temp->name)
        {
            fprintf(stderr, "error: lookup failed\n");
            return NULL;
        }

        if (strncmp(p_temp->name, name, UINT16_MAX) == 0)
        {
            break;
        }

        p_temp = p_temp->next;
    }

    if (!p_temp)
    {
        printf("lookup_user: user not found\n");
    }

    return p_temp;
} /* lookup_user */


/*
 * @brief Deletes a user from the table.
 *
 * @param p_table: Point to the table.
 * @param name: Name of user to be deleted.
 * @return true on success; false on failure.
 */
bool
delete_user (hash_table_t * p_table, const char * name)
{
    if (!p_table || !p_table->items || !name)
    {
        fprintf(stderr, "error: delete failed\n");
        return false;
    }

    uint64_t name_len = strlen(name);
    uint64_t idx = djb_hash(name, name_len) % p_table->capacity;

    if ((EINVAL == errno) && (0 == idx))
    {
        perror("unable to hash username.");
        return false;
    }

    user_t * p_temp = p_table->items[idx];
    user_t * p_prev = NULL;

    while (p_temp)
    {
        if (!p_temp->name)
        {
            fprintf(stderr, "error: delete failed\n");
            return false;
        }

        if (strncmp(p_temp->name, name, UINT16_MAX) == 0)
        {
            break;
        }

        p_prev = p_temp;
        p_temp = p_temp->next;
    }

    if (!p_temp)
    {
        printf("delete_user: user not found\n");
        return false;
    }

    if (!p_prev)
    {
        // Case where user is the head of the list.
        //
        p_table->items[idx] = p_temp->next;
    }
    else
    {
        p_prev->next = p_temp->next;
    }

    destroy_user(&p_temp);
    return true;
} /* delete_user */


/*
 * @brief Destroys the hash table.
 *
 * @param pp_table: Double pointer to the table.
 */
void
destroy_table (hash_table_t ** pp_table)
{
    if (!pp_table || !(*pp_table))
    {
        return;
    }

    if ((*pp_table)->items)
    {
        for (uint64_t idx = 0; idx < (*pp_table)->capacity; ++idx)
        {
            if ((*pp_table)->items[idx])
            {
                user_t * p_temp = (*pp_table)->items[idx];
                user_t * p_next = NULL;

                while (p_temp)
                {
                    p_next = p_temp->next;
                    destroy_user(&p_temp);
                    p_temp = p_next;
                }
            }
        }
    }

    free((*pp_table)->items);
    (*pp_table)->items = NULL;
    free(*pp_table);
    *pp_table = NULL;
    return;
} /* destroy_table */


/*
 * @brief When the current table hits the established load factor, this function
 * creates a new hash table and rehashes all entries in the old table and places
 * them in their appropriate positions in the newly created table. This is a
 * costly operation, but it is necessary to maintain the efficiency of the
 * table.
 *
 * @param pp_old_ht: Double pointer to the old table.
 * @return hash_table_t * of the newly created table on success; NULL on failure
 */
hash_table_t *
rehash_table (hash_table_t ** pp_old_ht)
{
    if (!pp_old_ht || !(*pp_old_ht))
    {
        fprintf(stderr, "error: rehashing failed\n");
        return NULL;
    }

    uint64_t new_cap = (*pp_old_ht)->capacity * 2;
    hash_table_t * p_new_ht = calloc(new_cap, sizeof(hash_table_t));

    if (!p_new_ht)
    {
        fprintf(stderr, "error: rehashing failed\n");
        return NULL;
    }

    p_new_ht->capacity = new_cap;
    p_new_ht->items = calloc(p_new_ht->capacity, sizeof(user_t *));

    if (!p_new_ht->items)
    {
        fprintf(stderr, "error: hash table creation failed\n");
        free(p_new_ht);
        p_new_ht = NULL;
        return p_new_ht;
    }

    if ((*pp_old_ht)->items)
    {
        for (uint64_t idx = 0; idx < (*pp_old_ht)->capacity; ++idx)
        {
            if ((*pp_old_ht)->items[idx])
            {
                user_t * p_temp = (*pp_old_ht)->items[idx];
                user_t * p_next = NULL;

                while (p_temp)
                {
                    p_next = p_temp->next;
                    insert_user(&p_new_ht, p_temp);
                    p_temp = p_next;
                }
            }
        }
    }

    free((*pp_old_ht)->items);
    (*pp_old_ht)->items = NULL;
    free(*pp_old_ht);
    *pp_old_ht = NULL;
    return p_new_ht;
} /* rehash_table */


/*
 * @brief Simple function that hashes a string and returns the result.
 * CITATION: http://www.partow.net/programming/hashfunctions/#RSHashFunction
 *
 * @param key The string to be hashed.
 * @param length The length of the provided string.
 * @return uint64_t The resultant hash on success; 0 on failure and sets errno
 * to EINVAL.
 */
static uint64_t
djb_hash(const char * key, uint64_t length)
{
    if (!key)
    {
        errno = EINVAL;
        return 0;
    }

    uint64_t hash = 5381;
    uint8_t shift = 5;

    for (uint64_t idx = 0; idx < length; ++key, ++idx)
    {
        hash = ((hash << shift) + hash) + (*key);
    }

    return hash;
} /* djb_hash */


/*** end of file ***/
