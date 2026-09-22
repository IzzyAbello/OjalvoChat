#include "message.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char* get_string_field(const cJSON* json, const char* field_name)
{
    cJSON *item = cJSON_GetObjectItemCaseSensitive(json, field_name);
    if (cJSON_IsString(item) && (item->valuestring != NULL))
        return strdup(item->valuestring);
    return NULL;
}

static Message_Type parse_message_type(const char* type_str)
{
    if (type_str == NULL) return MESSAGE_TYPE_UNKNOWN;
    if (strcmp(type_str, "IDENTIFY") == 0)         return MESSAGE_TYPE_IDENTIFY;
    if (strcmp(type_str, "STATUS") == 0)           return MESSAGE_TYPE_STATUS;
    if (strcmp(type_str, "USERS") == 0)            return MESSAGE_TYPE_USERS;
    if (strcmp(type_str, "TEXT") == 0)             return MESSAGE_TYPE_TEXT;
    if (strcmp(type_str, "PUBLIC_TEXT") == 0)      return MESSAGE_TYPE_PUBLIC_TEXT;
    if (strcmp(type_str, "NEW_ROOM") == 0)         return MESSAGE_TYPE_NEW_ROOM;
    if (strcmp(type_str, "INVITE") == 0)           return MESSAGE_TYPE_INVITE;
    if (strcmp(type_str, "JOIN_ROOM") == 0)        return MESSAGE_TYPE_JOIN_ROOM;
    if (strcmp(type_str, "ROOM_USERS") == 0)       return MESSAGE_TYPE_ROOM_USERS;
    if (strcmp(type_str, "ROOM_TEXT") == 0)        return MESSAGE_TYPE_ROOM_TEXT;
    if (strcmp(type_str, "LEAVE_ROOM") == 0)       return MESSAGE_TYPE_LEAVE_ROOM;
    if (strcmp(type_str, "DISCONNECT") == 0)       return MESSAGE_TYPE_DISCONNECT;
    if (strcmp(type_str, "NEW_STATUS") == 0)       return MESSAGE_TYPE_NEW_STATUS;
    if (strcmp(type_str, "USER_LIST") == 0)        return MESSAGE_TYPE_USER_LIST;
    if (strcmp(type_str, "TEXT_FROM") == 0)        return MESSAGE_TYPE_TEXT_FROM;
    if (strcmp(type_str, "PUBLIC_TEXT_FROM") == 0) return MESSAGE_TYPE_PUBLIC_TEXT_FROM;
    if (strcmp(type_str, "JOINED_ROOM") == 0)      return MESSAGE_TYPE_JOINED_ROOM;
    if (strcmp(type_str, "ROOM_USER_LIST") == 0)   return MESSAGE_TYPE_ROOM_USER_LIST;
    if (strcmp(type_str, "ROOM_TEXT_FROM") == 0)   return MESSAGE_TYPE_ROOM_TEXT_FROM;
    if (strcmp(type_str, "LEFT_ROOM") == 0)        return MESSAGE_TYPE_LEFT_ROOM;
    if (strcmp(type_str, "NEW_USER") == 0)         return MESSAGE_TYPE_NEW_USER;
    if (strcmp(type_str, "DISCONNECTED") == 0)     return MESSAGE_TYPE_DISCONNECTED;
    if (strcmp(type_str, "RESPONSE") == 0)         return MESSAGE_TYPE_RESPONSE;
    return MESSAGE_TYPE_UNKNOWN;
}

static char* message_type_to_string (const Message_Type type)
{
    if (type == MESSAGE_TYPE_IDENTIFY)         return strdup("IDENTIFY");
    if (type == MESSAGE_TYPE_STATUS)           return strdup("STATUS");
    if (type == MESSAGE_TYPE_USERS)            return strdup("USERS");
    if (type == MESSAGE_TYPE_TEXT)             return strdup("TEXT");
    if (type == MESSAGE_TYPE_PUBLIC_TEXT)      return strdup("PUBLIC_TEXT");
    if (type == MESSAGE_TYPE_NEW_ROOM)         return strdup("NEW_ROOM");
    if (type == MESSAGE_TYPE_INVITE)           return strdup("INVITE");
    if (type == MESSAGE_TYPE_JOIN_ROOM)        return strdup("JOIN_ROOM");
    if (type == MESSAGE_TYPE_ROOM_USERS)       return strdup("ROOM_USERS");
    if (type == MESSAGE_TYPE_ROOM_TEXT)        return strdup("ROOM_TEXT");
    if (type == MESSAGE_TYPE_LEAVE_ROOM)       return strdup("LEAVE_ROOM");
    if (type == MESSAGE_TYPE_DISCONNECT)       return strdup("DISCONNECT");
    if (type == MESSAGE_TYPE_NEW_STATUS)       return strdup("NEW_STATUS");
    if (type == MESSAGE_TYPE_USER_LIST)        return strdup("USER_LIST");
    if (type == MESSAGE_TYPE_TEXT_FROM)        return strdup("TEXT_FROM");
    if (type == MESSAGE_TYPE_PUBLIC_TEXT_FROM) return strdup("PUBLIC_TEXT_FROM");
    if (type == MESSAGE_TYPE_ROOM_TEXT_FROM)   return strdup("ROOM_TEXT_FROM");
    if (type == MESSAGE_TYPE_JOINED_ROOM)      return strdup("JOINED_ROOM");
    if (type == MESSAGE_TYPE_ROOM_USER_LIST)   return strdup("ROOM_USER_LIST");
    if (type == MESSAGE_TYPE_TEXT_FROM)        return strdup("TEXT_FROM");
    if (type == MESSAGE_TYPE_LEFT_ROOM)        return strdup("LEFT_ROOM");
    if (type == MESSAGE_TYPE_NEW_USER)         return strdup("NEW_USER");
    if (type == MESSAGE_TYPE_DISCONNECTED)     return strdup("DISCONNECTED");
    if (type == MESSAGE_TYPE_RESPONSE)         return strdup("RESPONSE");
    return strdup("UNKNOWN");
}

void message_init(Message* msg)
{
    if (msg == NULL) return;
    msg->type = MESSAGE_TYPE_UNKNOWN;
    msg->username = NULL;
    msg->operation = NULL;
    msg->result = NULL;
    msg->extra = NULL;
    msg->status = NULL;
    msg->users = NULL;
    msg->text = NULL;
    msg->roomname = NULL;
    msg->usernames = NULL;
    msg->usernames_count = 0;
}

void message_destroy(Message *msg)
{
    if (msg == NULL) return;

    free(msg->username);
    free(msg->operation);
    free(msg->result);
    free(msg->extra);
    free(msg->status);
    free(msg->text);
    free(msg->roomname);

    if (msg->users != NULL) cJSON_Delete(msg->users);

    if (msg->usernames != NULL)
    {
        for (size_t i = 0; i < msg->usernames_count; i++)
            free(msg->usernames[i]);
        free(msg->usernames);
    }

    message_init(msg);
}

bool message_from_json(const char* json_raw, Message* out_msg)
{
    if (json_raw == NULL || out_msg == NULL) return false;

    message_init(out_msg);

    cJSON* json = cJSON_Parse(json_raw);
    if (json == NULL) return false;

    cJSON *type_item = cJSON_GetObjectItemCaseSensitive(json, "type");
    if (cJSON_IsString(type_item) && (type_item->valuestring != NULL))
        out_msg->type = parse_message_type(type_item->valuestring);

    out_msg->username  = get_string_field(json, "username");
    out_msg->operation = get_string_field(json, "operation");
    out_msg->result    = get_string_field(json, "result");
    out_msg->extra     = get_string_field(json, "extra");
    out_msg->status    = get_string_field(json, "status");
    out_msg->text      = get_string_field(json, "text");
    out_msg->roomname  = get_string_field(json, "roomname");
    out_msg->users     = cJSON_DetachItemFromObject(json, "users");

    cJSON* usernames_array = cJSON_GetObjectItemCaseSensitive(json, "usernames");
    if (cJSON_IsArray(usernames_array))
    {
        int count = cJSON_GetArraySize(usernames_array);
        if (count > 0)
        {
            out_msg->usernames = malloc(sizeof(char *) * (size_t)count);
            if (out_msg->usernames != NULL)
            {
                out_msg->usernames_count = 0;
                for (int i = 0; i < count; i++)
                {
                    cJSON *elem = cJSON_GetArrayItem(usernames_array, i);
                    if (cJSON_IsString(elem) && (elem->valuestring != NULL))
                    {
                        out_msg->usernames[out_msg->usernames_count] = strdup(elem->valuestring);
                        out_msg->usernames_count++;
                    }
                }
            }
        }
    }

    cJSON_Delete(json);
    return true;
}

bool message_to_json(const Message* msg, char* out_json, size_t out_json_size)
{
    if (msg == NULL || out_json == NULL) return false;
 
    cJSON *json = cJSON_CreateObject();
    if (json == NULL) return false;
 
    cJSON_AddStringToObject(json, "type", message_type_to_string(msg->type));
 
    if (msg->username != NULL)  cJSON_AddStringToObject(json, "username", msg->username);
    if (msg->operation != NULL) cJSON_AddStringToObject(json, "operation", msg->operation);
    if (msg->result != NULL)    cJSON_AddStringToObject(json, "result", msg->result);
    if (msg->extra != NULL)     cJSON_AddStringToObject(json, "extra", msg->extra);
    if (msg->status != NULL)    cJSON_AddStringToObject(json, "status", msg->status);
    if (msg->text != NULL)      cJSON_AddStringToObject(json, "text", msg->text);
    if (msg->roomname != NULL)  cJSON_AddStringToObject(json, "roomname", msg->roomname);
    if (msg->users != NULL)     cJSON_AddItemToObject(json, "users", cJSON_Duplicate(msg->users, true));
 
    if (msg->usernames != NULL && msg->usernames_count > 0)
    {
        cJSON *usernames_array = cJSON_AddArrayToObject(json, "usernames");
        for (size_t i = 0; i < msg->usernames_count; i++)
            if (msg->usernames[i] != NULL)
                cJSON_AddItemToArray(usernames_array, cJSON_CreateString(msg->usernames[i]));
    }

    char *printed = cJSON_PrintUnformatted(json);
    cJSON_Delete(json);
    if (printed == NULL) return false;
 
    size_t length = strlen(printed);
    if (length + 1 > out_json_size)
    {
        cJSON_free(printed);
        return false;
    }
 
    memcpy(out_json, printed, length + 1);
    cJSON_free(printed);
    return true;
}