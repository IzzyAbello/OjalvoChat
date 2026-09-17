#ifndef USERS_TABLE_H
#define USERS_TABLE_H

#include <glib.h>
#include <stdbool.h>

#include "user.h"

typedef struct
{
    GHashTable* table;
    GMutex mutex;
}
Users_Table;

void users_table_init(Users_Table* table);

void users_table_destroy(Users_Table* table);

bool users_table_add(Users_Table* table, const char* username, int socket_fd);

bool users_table_find(const Users_Table* table, const char* username, User* out_user);

bool users_table_remove(Users_Table* table, const char* username);

#endif /* USERS_TABLE_H */