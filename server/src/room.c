#include "room.h"

#include <stdlib.h>
#include <string.h>

bool room_init(Room* room, const char* roomname)
{
    if (roomname == NULL) return false;

    size_t length = strlen(roomname);
    if (length == 0 || length > ROOM_ROOMNAME_MAX_LENGTH)
        return false;

    room->roomname = strdup(roomname);
    if (room->roomname == NULL) return false;

    users_table_init(&room->members);
    users_table_init(&room->guests);

    return true;
}

void room_destroy(Room* room)
{
    free(room->roomname);
    room->roomname = NULL;

    users_table_destroy(&room->members);
    users_table_destroy(&room->guests);
}