#include "users_table.h"

#include <stdlib.h>

void users_table_init(Users_Table* table)
{
    table->table = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, free);
    g_mutex_init(&table->mutex);
}

void users_table_destroy(Users_Table* table)
{
    g_hash_table_destroy(table->table);
    g_mutex_clear(&table->mutex);
}

bool users_table_add(Users_Table* table, const char* username, int socket_fd)
{
    User* user = malloc(sizeof(User));
    if (user == NULL) return false;

    if (!user_init(user, username, socket_fd))
    {
        free(user);
        return false;
    }

    g_mutex_lock(&table->mutex);

    if (g_hash_table_contains(table->table, username))
    {
        g_mutex_unlock(&table->mutex);
        free(user);
        return false;
    }

    g_hash_table_insert(table->table, g_strdup(username), user);

    g_mutex_unlock(&table->mutex);
    return true;
}

bool users_table_find(const Users_Table* table, const char* username, User* out_user)
{
    GMutex* mutex = (GMutex*)&table->mutex;

    g_mutex_lock(mutex);

    User* found = g_hash_table_lookup(table->table, username);
    bool exists = (found != NULL);
    
    if (exists) *out_user = *found;

    g_mutex_unlock(mutex);
    return exists;
}

bool users_table_remove(Users_Table *table, const char *username)
{
    g_mutex_lock(&table->mutex);
    bool removed = g_hash_table_remove(table->table, username);
    g_mutex_unlock(&table->mutex);
    return removed;
}