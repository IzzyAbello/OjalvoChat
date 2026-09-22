#include "room_table.h"

#include <stdlib.h>

static void room_free(gpointer data)
{
    Room* room = (Room*)data;
    room_destroy(room);
    free(room);
}

void room_table_init(Room_Table* table)
{
    table->table = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, room_free);
    g_mutex_init(&table->mutex);
}

void room_table_destroy(Room_Table* table)
{
    g_hash_table_destroy(table->table);
    g_mutex_clear(&table->mutex);
}

bool room_table_add(Room_Table* table, const char* roomname)
{
    Room* room = malloc(sizeof(Room));
    if (room == NULL) return false;

    if (!room_init(room, roomname))
    {
        free(room);
        return false;
    }

    g_mutex_lock(&table->mutex);

    if (g_hash_table_contains(table->table, roomname))
    {
        g_mutex_unlock(&table->mutex);
        room_destroy(room);
        free(room);
        return false;
    }

    g_hash_table_insert(table->table, g_strdup(roomname), room);

    g_mutex_unlock(&table->mutex);
    return true;
}

bool room_table_contains(const Room_Table* table, const char* roomname)
{
    GMutex* mutex = (GMutex*)&table->mutex;

    g_mutex_lock(mutex);
    bool exists = g_hash_table_contains(table->table, roomname);
    g_mutex_unlock(mutex);

    return exists;
}

bool room_table_with(Room_Table* table, const char* roomname, Room_Action action, void* context)
{
    g_mutex_lock(&table->mutex);

    Room* found = g_hash_table_lookup(table->table, roomname);
    
    if (found != NULL) action(found, context);

    g_mutex_unlock(&table->mutex);
    return found != NULL;
}

bool room_table_remove(Room_Table* table, const char* roomname)
{
    g_mutex_lock(&table->mutex);
    bool removed = g_hash_table_remove(table->table, roomname);
    g_mutex_unlock(&table->mutex);
    return removed;
}

typedef struct
{
    Room_Visitor visitor;
    void* context;
}
Room_For_Each_Adapter;

static void room_for_each_adapter(gpointer key, gpointer value, gpointer user_data)
{
    (void)key;
    Room_For_Each_Adapter* adapter = (Room_For_Each_Adapter*)user_data;
    adapter->visitor((Room*)value, adapter->context);
}

void room_table_for_each(Room_Table* table, Room_Visitor visitor, void* context)
{
    Room_For_Each_Adapter adapter = { .visitor = visitor, .context = context };

    g_mutex_lock(&table->mutex);
    g_hash_table_foreach(table->table, room_for_each_adapter, &adapter);
    g_mutex_unlock(&table->mutex);
}