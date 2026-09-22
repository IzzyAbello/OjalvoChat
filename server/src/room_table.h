#ifndef ROOM_TABLE_H
#define ROOM_TABLE_H

#include <glib.h>
#include <stdbool.h>

#include "room.h"

typedef struct
{
    GHashTable* table;
    GMutex mutex;
}
Room_Table;

void room_table_init(Room_Table* table);

void room_table_destroy(Room_Table* table);

bool room_table_add(Room_Table* table, const char* roomname);

bool room_table_contains(const Room_Table* table, const char* roomname);

typedef void (*Room_Action)(Room* room, void* context);

bool room_table_with(Room_Table* table, const char* roomname, Room_Action action, void* context);

bool room_table_remove(Room_Table* table, const char* roomname);

typedef void (*Room_Visitor)(Room* room, void* context);

void room_table_for_each(Room_Table* table, Room_Visitor visitor, void* context);

#endif /* ROOM_TABLE_H */