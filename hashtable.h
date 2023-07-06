#ifndef HASHTABLE_H
#define HASHTABLE_H

#include <stdbool.h>
#include <stdint.h>
#include <time.h>

// Table values
#define LOAD_FACTOR  0.75
#define INIT_TBL_CAP 64

typedef struct user
{
    char *        name;
    char *        passwd;
    uint32_t      sess_id;
    time_t        last_act_time;
    uint8_t       perms;
    struct user * next;
} user_t;

typedef struct
{
    user_t ** items;
    uint64_t  capacity;
    uint64_t  num_items;
} hash_table_t;


/*
 * @brief Creates a hash table and return a pointer to it.
 *
 * @return hash_table_t * on success; NULL on failure.
 */
hash_table_t * create_table (void);

/*
 * @brief Creates a user.
 *
 * @param: User's name.
 * @param: Length of user's name.
 * @param: User's password
 * @param: Length of user's password.
 * @param: User's permission level.
 * @return user_t * on success; NULL on failure.
 */
user_t * create_user (const char *, uint16_t, const char *, uint16_t, uint8_t);

/*
 * @brief Inserts a user item into the hashtable.
 *
 * In the event of a collision, this algorithm inserts the new user at the
 * beginning of the chain of users, in an effort to maintain efficiency
 * upon insertion.
 *
 * @param: Pointer to hashtable.
 * @param: Pointer to user to be added.
 * @return true on success; false on failure.
 */
bool insert_user (hash_table_t **, user_t *);

/*
 * @brief Locates an entry in the table by name.
 *
 * @param: Pointer to table.
 * @param: Name of potential user.
 * @return user_t * upon success; NULL on failure.
 */
user_t * lookup_user (hash_table_t *, const char *);

/*
 * @brief Deletes a user from the table.
 *
 * @param: Point to the table.
 * @param: Name of user to be deleted.
 * @return true on success; false on failure.
 */
bool delete_user (hash_table_t *, const char *);

/*
 * @brief Destroys the hash table.
 *
 * @param: Double pointer to the table.
 */
void destroy_table (hash_table_t **);

/*
 * @brief When the current table hits the established load factor, this function
 * creates a new hash table and rehashes all entries in the old table and places
 * them in their appropriate positions in the newly created table. This is a
 * costly operation, but it is necessary to maintain the efficiency of the
 * table.
 *
 * @param: Double pointer to the old table.
 * @return hash_table_t * of the newly created table on success; NULL on failure
 */
hash_table_t * rehash_table (hash_table_t **);


#endif /* HASHTABLE_H */

/*** end of file ***/

