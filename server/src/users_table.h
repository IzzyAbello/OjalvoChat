#ifndef USERS_TABLE_H
#define USERS_TABLE_H

#include <glib.h>
#include <stdbool.h>

#include "user.h"

typedef struct
{
    GHashTable* table;
    GHashTable* table_by_fd;
    GMutex mutex;
}
Users_Table;

void users_table_init(Users_Table* table);

void users_table_destroy(Users_Table* table);

bool users_table_add(Users_Table* table, const char* username, int socket_fd);

bool users_table_contains_by_username(Users_Table* table, const char* username);

bool users_table_contains_by_client_fd(Users_Table* table, const int client_fd);

bool users_table_find_by_username(const Users_Table* table, const char* username, User* out_user);

bool users_table_find_by_client_fd(const Users_Table* table, const int client_fd, User* out_user);

bool users_table_change_status_by_username(const Users_Table* table, const char* username, User_Status status);

bool users_table_change_status_by_client_fd(const Users_Table* table, const int client_fd, User_Status status);

bool users_table_remove(Users_Table* table, const char* username);

typedef struct
{
    GHashTableIter iter;
    Users_Table* table;
}
Users_Table_Iter;
 
void users_table_iter_begin(Users_Table* table, Users_Table_Iter* iter);
 
bool users_table_iter_next(Users_Table_Iter* iter, User* out_user);
 
void users_table_iter_end(Users_Table_Iter* iter);
 
typedef void (*User_Visitor)(User* user, void* context);
 
void users_table_for_each(Users_Table* table, User_Visitor visitor, void* context);

#endif /* USERS_TABLE_H */