#ifndef MESSAGE_H
#define MESSAGE_H

#include <stdbool.h>
#include <stddef.h>
#include <cjson/cJSON.h>

typedef enum
{
    MESSAGE_TYPE_IDENTIFY,
    MESSAGE_TYPE_STATUS,
    MESSAGE_TYPE_USERS,
    MESSAGE_TYPE_TEXT,
    MESSAGE_TYPE_PUBLIC_TEXT,
    MESSAGE_TYPE_NEW_ROOM,
    MESSAGE_TYPE_INVITE,
    MESSAGE_TYPE_JOIN_ROOM,
    MESSAGE_TYPE_ROOM_USERS,
    MESSAGE_TYPE_ROOM_TEXT,    
    MESSAGE_TYPE_LEAVE_ROOM,
    MESSAGE_TYPE_DISCONNECT,
    MESSAGE_TYPE_NEW_USER,
    MESSAGE_TYPE_NEW_STATUS,
    MESSAGE_TYPE_USER_LIST,
    MESSAGE_TYPE_TEXT_FROM,
    MESSAGE_TYPE_PUBLIC_TEXT_FROM,
    MESSAGE_TYPE_JOINED_ROOM,
    MESSAGE_TYPE_ROOM_USER_LIST,
    MESSAGE_TYPE_ROOM_TEXT_FROM,
    MESSAGE_TYPE_LEFT_ROOM,
    MESSAGE_TYPE_DISCONNECTED,
    MESSAGE_TYPE_RESPONSE,
    MESSAGE_TYPE_UNKNOWN
}
Message_Type;

typedef struct
{
    Message_Type type;
    char* username;
    char* operation;
    char* result;
    char* extra;
    char* status;
    cJSON* users;
    char* text;
    char* roomname;
    char** usernames;
    size_t usernames_count;
}
Message;

void message_init(Message* msg);

void message_destroy(Message* msg);

bool message_from_json(const char* json_raw, Message* out_msg);

bool message_to_json(const Message* msg, char* out_json, size_t out_json_size);

#endif // MESSAGE_H