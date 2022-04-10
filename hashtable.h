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




#endif /* HASHTABLE_H */

/*** end of file ***/

