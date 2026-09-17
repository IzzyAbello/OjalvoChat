#include "user.h"

#include <string.h>
 
bool user_init(User* user, const char* username, int socket_fd)
{
    if (username == NULL) return false;
 
    size_t length = strlen(username);

    if (length == 0 || length > USER_USERNAME_MAX_LENGTH) return false;
 
    memcpy(user->username, username, length);
    user->username[length] = '\0';
    user->socket_fd = socket_fd;
    user->status = USER_STATUS_ACTIVE;
    return true;
}
 
const char* user_status_to_string(User_Status status)
{
    switch (status)
    {
        case USER_STATUS_ACTIVE:
            return "ACTIVE";
        case USER_STATUS_AWAY:
            return "AWAY";
        case USER_STATUS_BUSY:
            return "BUSY";
        default:
            return "ACTIVE";
    }
}
 
bool user_status_from_string(const char* text, User_Status* out_status)
{
    if (text == NULL) return false;
 
    if (strcmp(text, "ACTIVE") == 0)
    {
        *out_status = USER_STATUS_ACTIVE;
        return true;
    }
    if (strcmp(text, "AWAY") == 0)
    {
        *out_status = USER_STATUS_AWAY;
        return true;
    }
    if (strcmp(text, "BUSY") == 0)
    {
        *out_status = USER_STATUS_BUSY;
        return true;
    }
 
    return false;
}