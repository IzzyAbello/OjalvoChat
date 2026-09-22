#ifndef ROOM_H
#define ROOM_H

#include <stdbool.h>

#include "users_table.h"

#define ROOM_ROOMNAME_MAX_LENGTH 16

typedef struct
{
    char* roomname;
    Users_Table members;
    Users_Table guests;
}
Room;

bool room_init(Room* room, const char* roomname);

void room_destroy(Room* room);

#endif /* ROOM_H */