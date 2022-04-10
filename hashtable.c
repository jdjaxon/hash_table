#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hashtable.h"

static uint64_t djb_hash(const char *, uint64_t);
static void destroy_user(user_t ** pp_user);
static void print_table(hash_table_t *);



/*** end of file ***/
